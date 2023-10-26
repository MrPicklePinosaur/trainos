#include <trainterm.h>
#include <trainstd.h>
#include <trainsys.h>
#include "render.h"
#include "user/nameserver.h"

typedef enum {
    RENDERER_APPEND_CONSOLE
} RendererMsgType;

typedef struct {
    RendererMsgType type;

    union {
        struct {
            char* line;
        } append_console;
    } data;
} RendererMsg;

typedef struct { } RendererResp;

int
renderer_append_console(Tid renderer_tid, char* line)
{
    RendererResp resp_buf;
    RendererMsg send_buf = (RendererMsg) {
        .type = RENDERER_APPEND_CONSOLE,
        .data = {
            .append_console = {
                .line = line
            }
        }
    };

    return Send(renderer_tid, (const char*)&send_buf, sizeof(RendererMsg), (char*)&resp_buf, sizeof(RendererResp));
}

void
renderTask()
{
    RegisterAs(RENDERER_ADDRESS);

    term_init();
    Window win = win_init(2, 2, 60, 30);
    win_draw(&win);

    usize console_length = 0;

    RendererMsg msg_buf;
    RendererResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(RendererMsg));

        if (msg_buf.type == RENDERER_APPEND_CONSOLE) {

            w_puts_mv(&win, msg_buf.data.append_console.line, 1, console_length);
            ++console_length;

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }

    }
}
