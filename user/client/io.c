#include <trainstd.h>
#include <traintasks.h>
#include "io.h"
#include "msg.h"
#include "user/trainstate.h"

typedef struct {
    ClientMsgType type;
    union {
        SensorClientMsg sensor;       
    } data;
} ClientIOMsg;

void
clientIoSensorNotifierTask()
{
    Tid client_io_server = MyParentTid();
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    for (;;) {
        Pair_usize_usize res = TrainstateWaitForSensor(trainstate_server, -1); 

        struct {} resp_buf;
        ClientIOMsg send_buf = (ClientIOMsg) {
            .type = CLIENT_MSG_SENSOR,
            .data = {
                .sensor = (SensorClientMsg) {
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

        if (msg_buf.type == CLIENT_MSG_SENSOR) {
            SensorClientMsg msg = msg_buf.data.sensor;
            client_send_msg(msg_buf.type, (const char*)&msg, sizeof(msg));
        }

        Reply(from_tid, (char*)&reply_buf, sizeof(reply_buf));
    }
    Exit();
}
