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
} RPSMove;

typedef enum {
    WIN,
    LOSE,
    TIE,
    INCOMPLETE, // the other player forfeit
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
                hashmap_insert(game_db, tid_to_key(player1), game_db);
                hashmap_insert(game_db, tid_to_key(player2), game_db);

                waiting_player = 0; // clear queue
            }

            reply_buf = (RPSResp) {
                .type = RPS_SIGNUP,
                .data = {
                    .signup = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(RPSResp));

        }
        else if (msg_buf.type == RPS_PLAY) {

            // look up the game the user to part of

            // check if the round is over

        }
        else if (msg_buf.type == RPS_QUIT) {

            // clean up the game the player is part of

            // inform other participant the game is over

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

}

int
Quit(Tid rps)
{

}

void
RPSClientTask(void)
{
    Tid rps = WhoIs(RPS_ADDRESS);
    Signup(rps);
    Exit();
}

void
RPSTask(void)
{
    Create(3, &RPSServerTask);
    for (unsigned int i = 0; i < 2; ++i) {
        Create(3, &RPSClientTask);
    }
    Yield();
    for (;;) {}
    Exit();
}
