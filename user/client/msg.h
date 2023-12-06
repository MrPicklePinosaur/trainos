#ifndef __CLIENT_MSG_H__
#define __CLIENT_MSG_H__

#include <trainstd.h>

typedef enum {
    CLIENT_MSG_SENSOR = 1,
} ClientMsgType;

typedef struct {
    usize train;
    usize sensor_id;
} SensorClientMsg;

void client_send_msg(ClientMsgType type, const char* msg, u32 size);

#endif // __CLIENT_MSG_H__
