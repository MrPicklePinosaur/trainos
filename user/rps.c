#include <trainsys.h>
#include <trainstd.h>

#include "usertasks.h"

#define RPS_ADDRESS "RPS"

typedef enum {
    RPS_SIGNUP,
    RPS_PLAY,
    RPS_QUIT,
} RPSMsgType;

typedef enum {
    MOVE_NONE = 0,
    MOVE_ROCK,
    MOVE_PAPER,
    MOVE_SCISSORS,
    MOVE_QUIT,
} RPSMove;

typedef enum {
    RESULT_INCOMPLETE = 0, // the other player forfeit
    RESULT_WIN,
    RESULT_LOSE,
    RESULT_TIE,
} RPSResult;

typedef struct {
    RPSMsgType type;

    union {
        struct { } signup;
        struct {
            RPSMove move;
        } play;
        struct { } quit;
    } data;
} RPSMsg;

typedef struct {
    RPSMsgType type;

    union {
        struct { } signup;
        struct {
            RPSResult res;
        } play;
        struct { } quit;
    } data;
} RPSResp;

typedef struct {
    Tid player1;
    Tid player2;
    RPSMove player1_move; 
    RPSMove player2_move; 
} RPSGameState;

// TID's would not need more than 64 bits
#define RPS_MAX_KEY_LEN 4

// convert tid to key usable with hashtable
char*
tid_to_key(Tid tid)
{
    char* key = alloc(sizeof(char)*RPS_MAX_KEY_LEN);
    ui2a(tid, 10, key);
    return key;
}

RPSResult
calculate_result(RPSMove player1_move, RPSMove player2_move)
{
    if (player1_move == MOVE_NONE || player2_move == MOVE_NONE) {
        println("WARNING, one move is none");
        return RESULT_INCOMPLETE;
    }

    // check if other player quit. player1_move should never be MOVE_QUIT
    if (player2_move == MOVE_QUIT) {
        return RESULT_INCOMPLETE;
    }

    // tie conditions
    if (player1_move == player2_move) return RESULT_TIE;

    // win conditions
    if (player1_move == MOVE_ROCK && player2_move == MOVE_SCISSORS) return RESULT_WIN;
    if (player1_move == MOVE_PAPER && player2_move == MOVE_ROCK) return RESULT_WIN;
    if (player1_move == MOVE_SCISSORS && player2_move == MOVE_PAPER) return RESULT_WIN;

    // the rest is lose conditions
    return RESULT_LOSE;
}

void
RPSServerTask(void)
{
    HashMap* game_db = hashmap_new(32);
    
    RPSMsg msg_buf;
    RPSResp reply_buf;

    Tid waiting_player = 0; // queued player

    // register self to nameserver
    RegisterAs(RPS_ADDRESS);
    Yield();

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(RPSMsg));
        if (msg_len < 0) {
            println("Error when receiving");
            continue;
        }

        if (msg_buf.type == RPS_SIGNUP) {

            if (waiting_player == 0) {
                // if there is not another player waiting to join, queue the player
                println("No other player in queue, adding player %d to queue", from_tid);
                waiting_player = from_tid;
            } else {
                // if there is a waiting player, we can start the game

                println("Player %d joined, player %d already in queue",from_tid, waiting_player);

                // create game object
                Tid player1 = waiting_player;
                Tid player2 = from_tid;
                RPSGameState* game_state = alloc(sizeof(RPSGameState));
                *game_state = (RPSGameState) {
                    .player1 = player1,
                    .player2 = player2,
                    .player1_move = MOVE_NONE,
                    .player2_move = MOVE_NONE,
                };

                // give both players a reference to the game state
                hashmap_insert(game_db, tid_to_key(player1), game_state);
                hashmap_insert(game_db, tid_to_key(player2), game_state);

                waiting_player = 0; // clear queue

                // reply to both players when ready to start
                reply_buf = (RPSResp) {
                    .type = RPS_SIGNUP,
                    .data = {
                        .signup = {}
                    }
                };
                Reply(player1, (char*)&reply_buf, sizeof(RPSResp));
                Reply(player2, (char*)&reply_buf, sizeof(RPSResp));
            }


        }
        else if (msg_buf.type == RPS_PLAY) {

            println("player %d played %d", from_tid, msg_buf.data.play.move);

            // look up the game the user to part of
            bool success;
            RPSGameState* game_state = hashmap_get(game_db, tid_to_key(from_tid), &success);
            if (!success) {
                println("couldn't find game state for player, maybe a game isn't started?");
                // TODO properly return error to client
                for(;;){}
            }

            println("got gamestate player1 = %d, player2 = %d, player1_move = %d, player2_move = %d", game_state->player1, game_state->player2, game_state->player1_move, game_state->player2_move);

            // TODO currently we are technically allowed to change our move as long as other player hasn't moved yet, consider if this is good behavior
            // need to figure out which player we are (yuck!)
            if (from_tid == game_state->player1) {
                game_state->player1_move = msg_buf.data.play.move;
            } else if (from_tid == game_state->player2) {
                game_state->player2_move = msg_buf.data.play.move;
            } else {
                println("player is not part of game");
                for(;;){}
            }

            // check if the round is over
            if (game_state->player1_move != MOVE_NONE && game_state->player2_move != MOVE_NONE) {
                // reply to both players with the result, or the non-quitting player if their opponent quit
                if (game_state->player1_move != MOVE_QUIT) {
                    reply_buf = (RPSResp) {
                        .type = RPS_PLAY,
                        .data = {
                            .play = {
                                .res = calculate_result(game_state->player1_move, game_state->player2_move)
                            }
                        }
                    };
                    Reply(game_state->player1, (char*)&reply_buf, sizeof(RPSResp));
                }

                if (game_state->player2_move != MOVE_QUIT) {
                    reply_buf = (RPSResp) {
                        .type = RPS_PLAY,
                        .data = {
                            .play = {
                                .res = calculate_result(game_state->player2_move, game_state->player1_move)
                            }
                        }
                    };
                    Reply(game_state->player2, (char*)&reply_buf, sizeof(RPSResp));
                }

                // reset the game state so players can play again
                game_state->player1_move = MOVE_NONE;
                game_state->player2_move = MOVE_NONE;
            }

        }
        else if (msg_buf.type == RPS_QUIT) {

            // TODO remove the game state from game_db

            bool success;
            RPSGameState* game_state = hashmap_get(game_db, tid_to_key(from_tid), &success);
            if (!success) {
                println("couldn't find game state for player, maybe a game isn't started?");
                // TODO properly return error to client
                for(;;){}
            }

            Tid other_tid;
            RPSMove* from_move;
            RPSMove* other_move;
            if (from_tid == game_state->player1) {
                other_tid = game_state->player2;
                from_move = &game_state->player1_move;
                other_move = &game_state->player2_move;
            }
            else {
                other_tid = game_state->player1;
                from_move = &game_state->player2_move;
                other_move = &game_state->player1_move;
            }

            // There are three cases to handle:

            // Case 1: Other player has not done anything yet, so store that you've quit
            if (*other_move == MOVE_NONE) {
                *from_move = MOVE_QUIT;
            }

            // Case 2: Other player has already made a move, reply to that move that you've quit
            else if (*other_move != MOVE_QUIT) {
                reply_buf = (RPSResp) {
                    .type = RPS_PLAY,
                    .data = {
                        .play = {
                            .res = RESULT_INCOMPLETE
                        }
                    }
                };
                Reply(other_tid, (char*)&reply_buf, sizeof(RPSResp));
            }

            // Case 3: Other player has also quit, and nothing needs to be done

            // Reply to quitting player's Send
            reply_buf = (RPSResp) {
                .type = RPS_QUIT,
                .data = {
                    .quit = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(RPSResp));

        } else {
            println("invalid message type");
            continue;
        }

    }

}

int
Signup(Tid rps)
{
    RPSResp resp_buf;
    RPSMsg send_buf = (RPSMsg) {
        .type = RPS_SIGNUP,
        .data = {
            .signup = {}
        }
    };

    int ret = Send(rps, (const char*)&send_buf, sizeof(RPSMsg), (char*)&resp_buf, sizeof(RPSResp));
    if (ret < 0) return -1;

    if (resp_buf.type != RPS_SIGNUP) return -1;

    println("successfully registered %d", MyTid()); 

    return 0;
}

RPSResult
Play(Tid rps, RPSMove move)
{
    RPSResp resp_buf;
    RPSMsg send_buf = (RPSMsg) {
        .type = RPS_PLAY,
        .data = {
            .play = {
                .move = move
            }
        }
    };

    int ret = Send(rps, (const char*)&send_buf, sizeof(RPSMsg), (char*)&resp_buf, sizeof(RPSResp));
    if (ret < 0) return -1;

    if (resp_buf.type != RPS_PLAY) return -1;

    return resp_buf.data.play.res;
}

int
Quit(Tid rps)
{
    RPSResp resp_buf;
    RPSMsg send_buf = (RPSMsg) {
        .type = RPS_QUIT,
        .data = {
            .quit = {}
        }
    };
    Send(rps, (const char*)&send_buf, sizeof(RPSMsg), (char*)&resp_buf, sizeof(RPSResp));
}

void
RPSClientTask1(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);
    RPSResult res = Play(rps, MOVE_ROCK);
    println("player %d played with result %d", MyTid(), res);
    res = Play(rps, MOVE_ROCK);
    println("player %d played with result %d", MyTid(), res);

    Quit(rps);
    println("player %d quit", MyTid());
    Exit();
}

void
RPSClientTask2(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);
    RPSResult res = Play(rps, MOVE_SCISSORS);
    println("player %d played with result %d", MyTid(), res);
    res = Play(rps, MOVE_PAPER);
    println("player %d played with result %d", MyTid(), res);
    res = Play(rps, MOVE_PAPER);
    println("player %d played with result %d", MyTid(), res);

    Quit(rps);
    println("player %d quit", MyTid());
    Exit();
}

void
RPSTask(void)
{
    Create(3, &RPSServerTask);
    Create(3, &RPSClientTask1);
    Create(3, &RPSClientTask2);
    Yield();
    for (;;) {}
    Exit();
}
