#include <trainsys.h>
#include <traintasks.h>
#include "sensor.h"
#include "marklin.h"

#define UNIT_COUNT 5
#define BYTE_COUNT 10
#define MAX_TRIGGERED 8

typedef enum {
    SENSOR_TRIGGERED,
    SENSOR_WAIT
} SensorMsgType;

typedef struct {
    SensorMsgType type;
    union {
        usize triggered[MAX_TRIGGERED+1]; // list of sensors that were triggered (-1 terminated array)
        usize wait;        // sensor id to wait for
    } data;
} SensorMsg;

typedef struct {
    SensorMsgType type;
    union {
        usize wait; // the sensor that was triggered
    } data;
} SensorResp;

int
WaitForSensor(Tid sensor_server, isize sensor)
{

    SensorResp resp_buf;
    SensorMsg send_buf = (SensorMsg) {
        .type = SENSOR_WAIT,
        .data = {
            .wait = sensor
        }
    };
    int ret = Send(sensor_server, (const char*)&send_buf, sizeof(SensorMsg), (char*)&resp_buf, sizeof(SensorResp));
    if (ret < 0) {
        ULOG_WARN("WaitForSensor errored");
        return -1;
    }
    return resp_buf.data.wait;
}

// task for querying sensor states
void
sensorNotifierTask() {
    u8 sensor_state[BYTE_COUNT] = {0};
    u8 prev_sensor_state[BYTE_COUNT] = {0};

    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = MyParentTid();

    for (;;) {

        marklin_dump_s88(marklin_server, UNIT_COUNT);

        for (usize i = 0; i < BYTE_COUNT; ++i) {
            u8 sensor_byte = Getc(marklin_server);
            prev_sensor_state[i] = sensor_state[i];
            sensor_state[i] = sensor_byte;
            u8 triggered = ~(prev_sensor_state[i]) & sensor_state[i];

            // send triggers in batches
            usize triggered_list[9];
            usize triggered_list_len = 0;
            for (usize j = 0; j < 8; ++j) {
                if (((triggered >> j) & 0x1) == 1) {
                    usize index = (7-j);

                    triggered_list[triggered_list_len] = i*8+index;
                    ++triggered_list_len;
                }
            }

            // send to server task
            if (triggered_list_len > 0) {
                ULOG_INFO_M(LOG_MASK_SENSOR, "sending sensor data to sensor server");
                triggered_list[triggered_list_len] = -1; // set element to be sentinel

                triggered_list_len = 0;

                SensorMsg send_buf = (SensorMsg) {
                    .type = SENSOR_TRIGGERED,
                    .data = {
                        .triggered = {0},
                    }
                };
                memcpy(send_buf.data.triggered, triggered_list, sizeof(triggered_list));
                SensorResp resp_buf;
                Send(sensor_server, (const char*)&send_buf, sizeof(SensorMsg), (char*)&resp_buf, sizeof(SensorResp));
            }
        }

        // TODO maybe should use DelayUntil to guarentee uniform fetches
        Delay(clock_server, 20);

    }

    Exit();
}

typedef struct {
    Tid tid;
    isize sensor_id;
} SensorRequest;

void
sensorServerTask()
{
    RegisterAs(SENSOR_ADDRESS); 

    Tid notifier = Create(2, &sensorNotifierTask, "Sensor Notifier");

    List* sensor_requests = list_init();

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
            ULOG_INFO_M(LOG_MASK_SENSOR, "[SENSOR SERVER] got sensor batch", *triggered);
            for (; *triggered != -1; ++triggered) {
                ULOG_INFO_M(LOG_MASK_SENSOR, "[SENSOR SERVER] triggered %d", *triggered);
                // unblock all tasks that are waiting

                ListIter it = list_iter(sensor_requests); 
                SensorRequest* request;
                while (listiter_next(&it, (void**)&request)) {

                    if (request->sensor_id == *triggered || request->sensor_id == -1) {
                        //ULOG_INFO_M(LOG_MASK_SENSOR, "[SENSOR SERVER] unblocking task %d", request->tid);
                        SensorResp reply_buf = (SensorResp) {
                            .type = SENSOR_WAIT,
                            .data = {
                                .wait = *triggered
                            }
                        };
                        Reply(request->tid, (char*)&reply_buf, sizeof(SensorResp));
                        list_remove(sensor_requests, request);
                    }

                }

            }

            SensorResp reply_buf = (SensorResp) {
                .type = SENSOR_TRIGGERED
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(SensorResp));
            
        }
        else if (msg_buf.type == SENSOR_WAIT) {
            ULOG_INFO_M(LOG_MASK_SENSOR, "[SENSOR SERVER] task %d request wait for sensor %d", from_tid, msg_buf.data.wait);

            SensorRequest* request = alloc(sizeof(SensorRequest));
            *request = (SensorRequest) {
                .tid = from_tid,
                .sensor_id = msg_buf.data.wait
            };
            list_push_back(sensor_requests, request);

            // Don't reply just yet

        } else {
            ULOG_WARN("[SENSOR SERVER] Invalid message type");
            continue;
        }

    }

    Exit();
}

str8
sensor_id_to_name(u8 id, Arena* arena)
{
    return str8_format(arena, "%c%d", id/16+'A', (id%16)+1);
}
