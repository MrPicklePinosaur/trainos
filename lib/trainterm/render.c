#include <trainstd.h>
#include <traintasks.h>
#include "render.h"

typedef Window* TraintermMsg;
typedef struct {} TraintermResp;

int
FlushWin(Tid trainterm_server, Window* win)
{
    TraintermResp resp_buf;
    TraintermMsg send_buf = win;
    int ret = Send(trainterm_server, (const char*)&send_buf, sizeof(TraintermMsg), (char*)&resp_buf, sizeof(TraintermResp));
    if (ret < 0) {
        ULOG_WARN("TrainterM errored");
        return -1;
    }
    return ret;
}

void
traintermTask()
{
    // round robin with a bucket for each task
    RegisterAs(TRAINTERM_ADDRESS);

    TraintermMsg msg_buf;
    TraintermResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(TraintermMsg));
        if (msg_len < 0) {
            ULOG_WARN("[TRAINTERM SERVER] Error when receiving");
            continue;
        }

        print(msg_buf->write_buffer);

        Reply(from_tid, (char*)&reply_buf, sizeof(TraintermResp));
    }
    Exit();
}
