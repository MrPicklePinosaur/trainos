#include <trainterm.h>
#include <trainstd.h>
#include <trainsys.h>
#include <ctype.h>
#include "render.h"
#include "ui.h"
#include "user/nameserver.h"

typedef enum {
    RENDERER_APPEND_CONSOLE,
    RENDERER_PROMPT, // rerender prompt
    RENDERER_SENSOR_TRIGGERED,
    RENDERER_SET_SWITCH,
} RendererMsgType;

typedef struct {
    RendererMsgType type;

    union {
        struct {
            char* line;
        } append_console;
        struct {
            char ch;
        } prompt;
        struct {
            usize sensor_id;
        } sensor_triggered;
        struct {
            usize switch_id;
            SwitchMode mode;
        } flip_switch;
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

int
renderer_prompt(Tid renderer_tid, char ch)
{
    RendererResp resp_buf;
    RendererMsg send_buf = (RendererMsg) {
        .type = RENDERER_PROMPT,
        .data = {
            .prompt = {
                .ch = ch
            }
        }
    };

    return Send(renderer_tid, (const char*)&send_buf, sizeof(RendererMsg), (char*)&resp_buf, sizeof(RendererResp));
}

int
renderer_sensor_triggered(Tid renderer_tid, usize sensor_id)
{
    RendererResp resp_buf;
    RendererMsg send_buf = (RendererMsg) {
        .type = RENDERER_SENSOR_TRIGGERED,
        .data = {
            .sensor_triggered = {
                .sensor_id = sensor_id
            }
        }
    };

    return Send(renderer_tid, (const char*)&send_buf, sizeof(RendererMsg), (char*)&resp_buf, sizeof(RendererResp));
}

int
renderer_flip_switch(Tid renderer_tid, usize switch_id, SwitchMode mode)
{
    RendererResp resp_buf;
    RendererMsg send_buf = (RendererMsg) {
        .type = RENDERER_SET_SWITCH,
        .data = {
            .flip_switch = {
                .switch_id = switch_id,
                .mode = mode
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

    Window prompt_win = win_init(2, 2, 60, 3);
    win_draw(&prompt_win);
    w_putc_mv(&prompt_win, '>', 1, 1);
    Window console_win = win_init(2, 5, 60, 30);
    win_draw(&console_win);
    Window sensor_win = win_init(63, 2, 20, 17);
    win_draw(&sensor_win);
    Window switch_win = win_init(63, 13, 20, 16);
    win_draw(&switch_win);

    usize console_length = 0;
    usize prompt_length = 0;

    RendererMsg msg_buf;
    RendererResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(RendererMsg));

        if (msg_buf.type == RENDERER_APPEND_CONSOLE) {

            w_puts_mv(&console_win, msg_buf.data.append_console.line, 1, 1+console_length);
            ++console_length;

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }
        else if (msg_buf.type == RENDERER_PROMPT) {

            const usize PROMPT_ANCHOR_X = 3;
            const usize PROMPT_ANCHOR_Y = 1;

            char ch = msg_buf.data.prompt.ch;
            if (ch == CH_BACKSPACE) {
                prompt_length = usize_sub(prompt_length, PROMPT_ANCHOR_Y);
                w_putc_mv(&prompt_win, ' ', PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
            }
            else if (ch == CH_ENTER) {
                for (usize i = 0; i < prompt_length; ++i) w_putc_mv(&prompt_win, ' ', PROMPT_ANCHOR_X+i, PROMPT_ANCHOR_Y);
                prompt_length = 0;
            }
            else if (isalnum(ch) || isblank(ch)) {
                // normal character
                // TODO max length for prompt
                w_putc_mv(&prompt_win, ch, PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
                prompt_length += 1;
            }

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }
        else if (msg_buf.type == RENDERER_SENSOR_TRIGGERED) {

            usize sensor_id = msg_buf.data.sensor_triggered.sensor_id;

            // update ui accordingly

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }


    }
}
