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
remove_game(HashMap* game_db, RPSGameState* game_state)
{
    Tid player1_tid = game_state->player1;
    Tid player2_tid = game_state->player2;

    bool result = hashmap_remove(game_db, tid_to_key(player1_tid));
    if (!result) {
        println("WARNING, when trying to remove game, couldn't find game state for player %d", player1_tid);
    }

    result = hashmap_remove(game_db, tid_to_key(player2_tid));
    if (!result) {
        println("WARNING, when trying to remove game, couldn't find game state for player %d", player2_tid);
    }
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
                println("Player %d joined, no players in queue", from_tid);
                waiting_player = from_tid;
            } else {
                // if there is a waiting player, we can start the game

                println("Player %d joined, player %d already in queue", from_tid, waiting_player);

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

            println("Player %d played %d", from_tid, msg_buf.data.play.move);

            // look up the game the user to part of
            bool success;
            RPSGameState* game_state = hashmap_get(game_db, tid_to_key(from_tid), &success);
            if (!success) {
                println("Couldn't find game state for player %d, maybe a game isn't started?", from_tid);
                // TODO properly return error to client
                for(;;){}
            }

            // TODO currently we are technically allowed to change our move as long as other player hasn't moved yet, consider if this is good behavior
            // need to figure out which player we are (yuck!)
            if (from_tid == game_state->player1) {
                game_state->player1_move = msg_buf.data.play.move;
            } else if (from_tid == game_state->player2) {
                game_state->player2_move = msg_buf.data.play.move;
            } else {
                println("Player is not part of this game");
                for(;;){}
            }

            // check if the round is over
            if (game_state->player1_move != MOVE_NONE && game_state->player2_move != MOVE_NONE) {
                println("Replying game results for player %d and player %d's game", game_state->player1, game_state->player2);

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

                if (game_state->player1_move == MOVE_QUIT || game_state->player2_move == MOVE_QUIT) {
                    remove_game(game_db, game_state);
                }

                // reset the game state so players can play again
                game_state->player1_move = MOVE_NONE;
                game_state->player2_move = MOVE_NONE;
            }

        }
        else if (msg_buf.type == RPS_QUIT) {
            bool success;
            RPSGameState* game_state = hashmap_get(game_db, tid_to_key(from_tid), &success);
            if (!success) {
                println("Couldn't find game state for player %d, maybe a game isn't started?", from_tid);
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

                remove_game(game_db, game_state);
            }

            // Case 3: Other player has also quit
            else {
                remove_game(game_db, game_state);
            }

            // Reply to quitting player's Send
            reply_buf = (RPSResp) {
                .type = RPS_QUIT,
                .data = {
                    .quit = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(RPSResp));

        } else {
            println("Invalid message type");
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
    if (ret < 0) {
        println("WARNING, player %d's Signup()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != RPS_SIGNUP) {
        println("WARNING, the reply to player %d's Signup()'s Send() call is not the right type", MyTid());
        return -1;
    }

    println("Player %d got a successful reply to their signup request", MyTid());

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
    if (ret < 0) {
        println("WARNING, player %d's Play()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != RPS_PLAY) {
        println("WARNING, the reply to player %d's Play()'s Send() call is not the right type", MyTid());
        return -1;
    }

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
    int ret = Send(rps, (const char*)&send_buf, sizeof(RPSMsg), (char*)&resp_buf, sizeof(RPSResp));
    if (ret < 0) {
        println("WARNING, player %d's Quit()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != RPS_QUIT) {
        println("WARNING, the reply to player %d's Quit()'s Send() call is not the right type", MyTid());
        return -1;
    }
    return 0;
}

void
RPSClientTask1(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);
    RPSResult res = Play(rps, MOVE_ROCK);
    println("Player %d got game result %d", MyTid(), res);
    res = Play(rps, MOVE_ROCK);
    println("Player %d got game result %d", MyTid(), res);

    Quit(rps);
    println("Player %d quit", MyTid());
    Exit();
}

void
RPSClientTask2(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);
    RPSResult res = Play(rps, MOVE_SCISSORS);
    println("Player %d got game result %d", MyTid(), res);
    res = Play(rps, MOVE_PAPER);
    println("Player %d got game result %d", MyTid(), res);

    Quit(rps);
    println("Player %d quit", MyTid());
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
