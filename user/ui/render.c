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

void
renderTask()
{
    RegisterAs(RENDERER_ADDRESS);

    term_init();

    Window prompt_win = win_init(2, 2, 60, 3);
    win_draw(&prompt_win);
    Window console_win = win_init(2, 5, 60, 30);
    win_draw(&console_win);

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

            char ch = msg_buf.data.prompt.ch;
            if (ch == CH_BACKSPACE) {
                prompt_length = usize_sub(prompt_length, 1);
                w_putc_mv(&prompt_win, ' ', 1+prompt_length, 1);
            }
            else if (ch == CH_ENTER) {
                w_mv(&prompt_win, 1, 1);
                for (usize i = 0; i < prompt_length; ++i) w_putc(&prompt_win, ' ');
                prompt_length = 0;
            }
            else if (isalnum(ch) || isblank(ch)) {
                // normal character
                // TODO max length for prompt
                w_putc_mv(&prompt_win, ch, 1+prompt_length, 1);
                prompt_length += 1;
            }

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }

    }
}
