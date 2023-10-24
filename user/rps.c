#include <trainsys.h>
#include <trainstd.h>

#include "nameserver.h"

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
    bool is_over;
    Tid player1;
    Tid player2;
    RPSMove player1_move; 
    RPSMove player2_move; 
} RPSGameState;

// TID's would not need more than 64 bits
#define RPS_MAX_KEY_LEN 4

static const char* const MOVE_TO_STRING[4] = {
    "NONE",
    "ROCK",
    "PAPER",
    "SCISSORS"
};

static const char* const RESULT_TO_STRING[4] = {
    "OPPONENT QUIT",
    "WIN",
    "LOSE",
    "TIE"
};

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
        println("[RPS SERVER] WARNING, one move is none");
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
            println("[RPS SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == RPS_SIGNUP) {

            if (waiting_player == 0) {
                // if there is not another player waiting to join, queue the player
                println("[RPS SERVER] Player %d joined, no players in queue", from_tid);
                waiting_player = from_tid;
            } else {
                // if there is a waiting player, we can start the game

                println("[RPS SERVER] Player %d joined, player %d in queue, starting game", from_tid, waiting_player);

                // create game object
                Tid player1 = waiting_player;
                Tid player2 = from_tid;
                RPSGameState* game_state = alloc(sizeof(RPSGameState));
                *game_state = (RPSGameState) {
                    .is_over = false,
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

            println("[RPS SERVER] Player %d played %s", from_tid, MOVE_TO_STRING[msg_buf.data.play.move]);

            // look up the game the user to part of
            bool success;
            RPSGameState* game_state = hashmap_get(game_db, tid_to_key(from_tid), &success);
            if (!success) {
                println("[RPS SERVER] Couldn't find game state for player %d, maybe a game isn't started?", from_tid);
                // TODO properly return error to client
                for(;;){}
            }

            // If the other player has quit, reply with the INCOMPLETE result
            if (game_state->is_over) {
                println("[RPS SERVER] Rejected player %d's play since the game is already over", from_tid);

                reply_buf = (RPSResp) {
                    .type = RPS_PLAY,
                    .data = {
                        .play = {
                            .res = RESULT_INCOMPLETE
                        }
                    }
                };
                Reply(from_tid, (char*)&reply_buf, sizeof(RPSResp));
            }

            // Otherwise, play the game
            else {
                // TODO currently we are technically allowed to change our move as long as other player hasn't moved yet, consider if this is good behavior
                // need to figure out which player we are (yuck!)
                if (from_tid == game_state->player1) {
                    game_state->player1_move = msg_buf.data.play.move;
                } else if (from_tid == game_state->player2) {
                    game_state->player2_move = msg_buf.data.play.move;
                } else {
                    println("[RPS SERVER] Player is not part of this game");
                    for(;;){}
                }

                // check if both players have made a move
                if (game_state->player1_move != MOVE_NONE && game_state->player2_move != MOVE_NONE) {
                    println("[RPS SERVER] Replying results for player %d and player %d's game", game_state->player1, game_state->player2);

                    reply_buf = (RPSResp) {
                        .type = RPS_PLAY,
                        .data = {
                            .play = {
                                .res = calculate_result(game_state->player1_move, game_state->player2_move)
                            }
                        }
                    };
                    Reply(game_state->player1, (char*)&reply_buf, sizeof(RPSResp));

                    reply_buf = (RPSResp) {
                        .type = RPS_PLAY,
                        .data = {
                            .play = {
                                .res = calculate_result(game_state->player2_move, game_state->player1_move)
                            }
                        }
                    };
                    Reply(game_state->player2, (char*)&reply_buf, sizeof(RPSResp));

                    // reset the game state so players can play again
                    game_state->player1_move = MOVE_NONE;
                    game_state->player2_move = MOVE_NONE;
                }
            }
        }
        else if (msg_buf.type == RPS_QUIT) {
            println("[RPS SERVER] Player %d requested to quit", from_tid);

            bool success;
            RPSGameState* game_state = hashmap_get(game_db, tid_to_key(from_tid), &success);
            if (!success) {
                println("[RPS SERVER] Couldn't find game state for player %d, maybe a game isn't started?", from_tid);
                // TODO properly return error to client
                for(;;){}
            }

            game_state->is_over = true;
            // Remember that games are stored under two TIDs, one per player.
            // We only remove the game under the TID of the quitting player.
            // Thus, the game is not fully removed from the hash map until both players have quit.
            bool result = hashmap_remove(game_db, tid_to_key(from_tid));
            if (!result) {
                println("[RPS SERVER] WARNING, when trying to remove game, couldn't find game state for player %d", from_tid);
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
            println("[RPS SERVER] Invalid message type");
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

    println("[RPS PLAYER %d] Requesting sign up", MyTid());

    int ret = Send(rps, (const char*)&send_buf, sizeof(RPSMsg), (char*)&resp_buf, sizeof(RPSResp));
    if (ret < 0) {
        println("[RPS PLAYER %d] WARNING, Signup()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != RPS_SIGNUP) {
        println("[RPS PLAYER %d] WARNING, the reply to Signup()'s Send() call is not the right type", MyTid());
        return -1;
    }

    println("[RPS PLAYER %d] Successfully signed up", MyTid());

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

    println("[RPS PLAYER %d] Playing move %s", MyTid(), MOVE_TO_STRING[send_buf.data.play.move]);

    int ret = Send(rps, (const char*)&send_buf, sizeof(RPSMsg), (char*)&resp_buf, sizeof(RPSResp));
    if (ret < 0) {
        println("[RPS PLAYER %d] WARNING, Play()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != RPS_PLAY) {
        println("[RPS PLAYER %d] WARNING, the reply to Play()'s Send() call is not the right type", MyTid());
        return -1;
    }

    println("[RPS PLAYER %d] Got game result %s", MyTid(), RESULT_TO_STRING[resp_buf.data.play.res]);

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

    println("[RPS PLAYER %d] Requesting to quit", MyTid());

    int ret = Send(rps, (const char*)&send_buf, sizeof(RPSMsg), (char*)&resp_buf, sizeof(RPSResp));
    if (ret < 0) {
        println("[RPS PLAYER %d] WARNING, Quit()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != RPS_QUIT) {
        println("[RPS PLAYER %d] WARNING, the reply to Quit()'s Send() call is not the right type", MyTid());
        return -1;
    }

    println("[RPS PLAYER %d] Successfully quit", MyTid());

    return 0;
}

void
RPSClientTask1(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);

    Play(rps, MOVE_ROCK);
    Play(rps, MOVE_ROCK);
    Play(rps, MOVE_ROCK);

    Quit(rps);
    Exit();
}

void
RPSClientTask2(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);

    Play(rps, MOVE_SCISSORS);
    Play(rps, MOVE_PAPER);
    Play(rps, MOVE_ROCK);
    Play(rps, MOVE_PAPER);
    Play(rps, MOVE_SCISSORS);

    Quit(rps);
    Exit();
}

void
RPSClientTask3(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);
    Play(rps, MOVE_ROCK);
    Quit(rps);

    Signup(rps);
    Play(rps, MOVE_SCISSORS);
    Quit(rps);

    Exit();
}

void
RPSTask(void)
{
    Create(3, &RPSServerTask, "RPS Server");

    println("");
    println("======================");
    println("======= TEST 1 =======");
    println("======================");
    Create(3, &RPSClientTask1, "RPS Test 1 AI-A");
    Create(3, &RPSClientTask2, "RPS Test 1 AI-B");
    Yield();

    println("");
    println("======================");
    println("======= TEST 2 =======");
    println("======================");
    Create(3, &RPSClientTask1, "RPS Test 2 AI-A #1");
    Create(3, &RPSClientTask2, "RPS Test 2 AI-B #1");
    Create(3, &RPSClientTask1, "RPS Test 2 AI-A #2");
    Create(3, &RPSClientTask1, "RPS Test 2 AI-A #3");
    Create(3, &RPSClientTask2, "RPS Test 2 AI-B #2");
    Create(3, &RPSClientTask2, "RPS Test 2 AI-B #3");
    Yield();

    println("");
    println("======================");
    println("======= TEST 3 =======");
    println("======================");
    Create(3, &RPSClientTask3, "RPS Test 3 AI-C #1");
    Create(3, &RPSClientTask3, "RPS Test 3 AI-C #2");
    Create(3, &RPSClientTask3, "RPS Test 3 AI-C #3");

    Exit();
}
