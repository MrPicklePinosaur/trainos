#include <trainsys.h>
#include "sensor.h"
#include "marklin.h"
#include "nameserver.h"
#include "io.h"
#include "clock.h"
#include "ui/render.h"

#define BYTE_COUNT 10
#define UNIT_COUNT 5

typedef enum {
    SENSOR_TRIGGERED,
    SENSOR_WAIT
} SensorMsgType;

typedef struct {
    SensorMsgType type;
    union {
        usize triggered[9]; // list of sensors that were triggered (0 terminated array)
        usize wait;        // sensor id to wait for
    } data;
} SensorMsg;

typedef struct {

} SensorResp;

// task for querying sensor states
void
sensorNotifierTask() {
    u8 sensor_state[BYTE_COUNT] = {0};
    u8 prev_sensor_state[BYTE_COUNT] = {0};

    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    //Tid renderer_server = WhoIs(RENDERER_ADDRESS);
    Tid sensor_server = MyParentTid();

    for (;;) {
        marklin_dump_s88(marklin_server, UNIT_COUNT);

        for (usize i = 0; i < BYTE_COUNT; ++i) {
            u8 sensor_byte = Getc(marklin_server);
            prev_sensor_state[i] = sensor_state[i];
            sensor_state[i] = sensor_byte;
            u8 triggered = ~(prev_sensor_state[i]) & sensor_state[i];

            // send triggers in batches
            usize triggered_list[9] = {0}; 
            usize triggered_list_len = 0;
            for (usize j = 0; j < 8; ++j) {
                if (((triggered >> j) & 0x1) == 1) {
                    usize index = (7-j);

                    triggered_list[triggered_list_len] = i*8+index;
                    ++triggered_list_len;

                    // TODO renderer should pull data instead of be pushed to
                    //renderer_sensor_triggered(renderer_server, i*8+index);
                }
            }

            // send to server task
            if (triggered_list_len > 0) {
                SensorMsg msg_buf = (SensorMsg) {
                    .type = SENSOR_TRIGGERED,
                    .data = {
                        .triggered = triggered_list
                    }
                };
                SensorResp reply_buf;
                
            }
        }

        // TODO maybe should use DelayUntil to guarentee uniform fetches
        Delay(clock_server, 20);
    }

    Exit();
}

void
sensorServerTask()
{
    
    Tid notifier = Create(2, &sensorNotifierTask, "Sensor Notifier");

    SensorMsg msg_buf;
    SensorResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(SensorMsg));
        if (msg_len < 0) {
            ULOG_WARN("[SENSOR SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == SENSOR_TRIGGERED) {

            // read sensor values and reply to anyone waiting

            usize* triggered = msg_buf.data.triggered;
            for (; triggered != 0; ++triggered) {
                ULOG_INFO("[SENSOR SERVER] triggered %d", *triggered);
            }

            Reply(from_tid, (char*)&reply_buf, sizeof(SensorResp));
            
        }
        else if (msg_buf.type == SENSOR_WAIT) {

        } else {
            ULOG_WARN("[SENSOR SERVER] Invalid message type");
            continue;
        }

    }

    Exit();
}
