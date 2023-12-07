#include <trainstd.h>
#include <traintasks.h>
#include "io.h"
#include "msg.h"
#include "user/trainstate.h"
#include "kern/dev/uart.h"

typedef struct {
    ClientMsgType type;
    union {
        SensorSendClientMsg send_sensor;
        
        TrainSpeedRecvClientMsg recv_train_speed;  
    } data;
} ClientIOMsg;

// receive data from the client
void
clientIoReceiveTask()
{
    const usize START_MSG_BYTES = 2; // magic bytes 0d6969
    const usize LENGTH_BYTES = 4; // u32
    const usize TYPE_BYTES = 4; // u32

    Tid client_io_server = MyParentTid();
    Tid io_server = WhoIs(IO_ADDRESS_CONSOLE);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    CBuf* input_buf = cbuf_new(256);
    for (;;) {
        
        u8 c = Getc(io_server);

        PRINT("got char %d", c);

        cbuf_push_back(input_buf, (void*)c);

        usize input_len = cbuf_len(input_buf);
        if (input_len < START_MSG_BYTES + LENGTH_BYTES + TYPE_BYTES) {
            PRINT("not full length yet");
            continue;
        }

        // invalid header
        if ((u8)cbuf_get(input_buf, 0) != 69 || (u8)cbuf_get(input_buf, 1) != 69) {
            PANIC("invalid header %d%d", cbuf_get(input_buf, 0), cbuf_get(input_buf, 1));
        }

        // TODO this may be incorrect
        PRINT("before shift");
        u32 msg_len = ((u8)cbuf_get(input_buf, 5) << 24)|((u8)cbuf_get(input_buf, 4) << 16)|((u8)cbuf_get(input_buf, 3) << 8)|((u8)cbuf_get(input_buf, 2) << 0);

        u32 msg_type = ((u8)cbuf_get(input_buf, 9) << 24)|((u8)cbuf_get(input_buf, 8) << 16)|((u8)cbuf_get(input_buf, 7) << 8)|((u8)cbuf_get(input_buf, 6) << 0);

        PRINT("msg_len = %d, msg_type = %d", msg_len, msg_type);

        if (input_len < START_MSG_BYTES + LENGTH_BYTES + TYPE_BYTES + msg_len) {
            continue;
        }
        
        // parse the command if we are done
        struct {} resp_buf;
        ClientIOMsg send_buf = (ClientIOMsg) {
            .type = msg_type,
        };

        char copied[256] = {0};
        for (usize i = START_MSG_BYTES+LENGTH_BYTES+TYPE_BYTES; i < input_len; ++i) {
            copied[i-(START_MSG_BYTES+LENGTH_BYTES+TYPE_BYTES)] = (char)cbuf_get(input_buf, i);
            PRINT("byte %d: %d", i, copied[i]);
        }

        switch (msg_type) {
            case CLIENT_MSG_RECV_TRAIN_SPEED:
                memcpy(&send_buf.data.recv_train_speed, copied, sizeof(TrainSpeedRecvClientMsg));
                /* send_buf.data.recv_train_speed = *(TrainSpeedRecvClientMsg*)copied; */
                PRINT("sending train speed: train = %d, speed = %d", send_buf.data.recv_train_speed.train, send_buf.data.recv_train_speed.speed);
                Send(client_io_server, (const char*)&send_buf, sizeof(ClientIOMsg), (char*)&resp_buf, sizeof(resp_buf));
                break;
            default: {}
        }

        // remove the parts that we have already parsed
        for (usize i = 0; i < input_len; ++i) {
            cbuf_pop_front(input_buf);
        }
    }
    Exit();
}

void
clientIoSensorNotifierTask()
{
    Tid client_io_server = MyParentTid();
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    for (;;) {
        Pair_usize_usize res = TrainstateWaitForSensor(trainstate_server, -1); 

        struct {} resp_buf;
        ClientIOMsg send_buf = (ClientIOMsg) {
            .type = CLIENT_MSG_SEND_SENSOR,
            .data = {
                .send_sensor = (SensorSendClientMsg) {
                    .train = res.first,
                    .sensor_id = res.second
                }
            }
        };
        Send(client_io_server, (const char*)&send_buf, sizeof(ClientIOMsg), (char*)&resp_buf, sizeof(resp_buf));
    }
}

void
clientIoTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    Create(5, &clientIoReceiveTask, "client io recieve task");
    Create(5, &clientIoSensorNotifierTask, "client io sensor notifier");

    ClientIOMsg msg_buf;
    struct {} reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(ClientIOMsg));
        if (msg_len < 0) {
            ULOG_WARN("[CLIENT IO SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == CLIENT_MSG_SEND_SENSOR) {
            SensorSendClientMsg msg = msg_buf.data.send_sensor;
            client_send_msg(msg_buf.type, (const char*)&msg, sizeof(msg));
        } else if (msg_buf.type == CLIENT_MSG_RECV_TRAIN_SPEED) {
            PRINT("got message to set train %d to speed %d", msg_buf.data.recv_train_speed.train, msg_buf.data.recv_train_speed.speed);
            TrainstateSetSpeed(trainstate_server, msg_buf.data.recv_train_speed.train, msg_buf.data.recv_train_speed.speed);
        }

        Reply(from_tid, (char*)&reply_buf, sizeof(reply_buf));
    }
    Exit();
}
