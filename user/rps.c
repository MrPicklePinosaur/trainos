#include <trainsys.h>
#include <trainstd.h>

#include "usertasks.h"

typedef enum {
    RPS_SIGNUP,
    RPS_PLAY,
    RPS_QUIT,
} RPSMsgType;

typedef enum {
    NONE = 0,
    ROCK,
    PAPER,
    SCISSORS,
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

#define RPS_MAX_GAMES 32
void
RPSTask(void)
{
    
    RPSMsg msg_buf;
    RPSResp reply_buf;

    Tid waiting_player = 0; // queued player

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
                waiting_player = from_tid;
            } else {
                // if there is a waiting player, we can start the game

                waiting_player = 0; // clear queue
            }

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
