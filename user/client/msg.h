#ifndef __CLIENT_MSG_H__
#define __CLIENT_MSG_H__

#include <trainstd.h>

typedef enum {
    CLIENT_MSG_SEND_SENSOR       = 1 << 0,

    CLIENT_MSG_RECV_TRAIN_SPEED  = 1 << 8, 
} ClientMsgType;

typedef struct {
    usize train;
    usize sensor_id;
} SensorSendClientMsg;

typedef struct {
    usize train;
    usize speed;
} TrainSpeedRecvClientMsg;

void client_send_msg(ClientMsgType type, const char* msg, u32 size);

#endif // __CLIENT_MSG_H__
