#ifndef __CLIENT_MSG_H__
#define __CLIENT_MSG_H__

#include <trainstd.h>

typedef enum {
    CLIENT_MSG_SEND_SENSOR       = 1 << 0,
    CLIENT_MSG_SEND_SWITCH       = 1 << 1,

    CLIENT_MSG_RECV_TRAIN_SPEED  = 1 << 8, 
    CLIENT_MSG_RECV_SWITCH       = 1 << 9, 
} ClientMsgType;

typedef struct {
    usize train;
    usize sensor_id;
} SensorSendClientMsg;

typedef struct {
    u16 state[5];
} SwitchSendClientMsg;

typedef struct {
    usize train;
    usize speed;
} TrainSpeedRecvClientMsg;

typedef struct {
    usize switch_id;
    bool state;
} SwitchRecvClientMsg;

void client_send_msg(ClientMsgType type, const char* msg, u32 size);

#endif // __CLIENT_MSG_H__
