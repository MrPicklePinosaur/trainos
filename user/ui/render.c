#include <trainterm.h>
#include <trainstd.h>
#include <trainsys.h>
#include <ctype.h>
#include "render.h"
#include "ui.h"
#include "user/nameserver.h"
#include "user/sensor.h"

typedef enum {
    RENDERER_APPEND_CONSOLE,
    RENDERER_PROMPT, // rerender prompt
    RENDERER_FLIP_SWITCH,
    RENDERER_DIAGNOSTIC,
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
            usize switch_id;
            SwitchMode mode;
        } flip_switch;
        struct {
            usize ticks;
            usize idle_percent;
        } diagnostic;
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
renderer_flip_switch(Tid renderer_tid, usize switch_id, SwitchMode mode)
{
    RendererResp resp_buf;
    RendererMsg send_buf = (RendererMsg) {
        .type = RENDERER_FLIP_SWITCH,
        .data = {
            .flip_switch = {
                .switch_id = switch_id,
                .mode = mode
            }
        }
    };

    return Send(renderer_tid, (const char*)&send_buf, sizeof(RendererMsg), (char*)&resp_buf, sizeof(RendererResp));
}

int
renderer_diagnostic(Tid renderer_tid, usize ticks, usize idle_percent)
{

    RendererResp resp_buf;
    RendererMsg send_buf = (RendererMsg) {
        .type = RENDERER_DIAGNOSTIC,
        .data = {
            .diagnostic = {
                .ticks = ticks,
                .idle_percent = idle_percent
            }
        }
    };

    return Send(renderer_tid, (const char*)&send_buf, sizeof(RendererMsg), (char*)&resp_buf, sizeof(RendererResp));
}

void
renderSensorWinTask()
{
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);

    const usize SENSOR_LIST_ANCHOR_X = 1;
    const usize SENSOR_LIST_ANCHOR_Y = 1;
    const Attr SENSOR_COLORS[5] = {ATTR_RED, ATTR_YELLOW, ATTR_GREEN, ATTR_CYAN, ATTR_MAGENTA};
    const usize MAX_SENSORS = 15;
    CBuf* triggered_sensors = cbuf_new(MAX_SENSORS);
    Window sensor_win = win_init(63, 6, 20, 17);
    win_draw(&sensor_win);
    w_puts_mv(&sensor_win, "[sensors]", 2, 0);

    for (;;) {

        usize next_sensor_id = WaitForSensor(sensor_server, -1);
        
        if (cbuf_len(triggered_sensors) >= MAX_SENSORS) {
            cbuf_pop_back(triggered_sensors);
        }
        cbuf_push_front(triggered_sensors, next_sensor_id);

        for (usize i = 0; i < min(MAX_SENSORS, cbuf_len(triggered_sensors)); ++i) {
            // build string from raw sensor id
            usize sensor_id = cbuf_get(triggered_sensors, i);

            usize sensor_group = sensor_id / 16;
            usize sensor_index = (sensor_id % 16) + 1;
            w_mv(&sensor_win, SENSOR_LIST_ANCHOR_X, SENSOR_LIST_ANCHOR_Y+i);
            w_puts(&sensor_win, "   ");
            w_mv(&sensor_win, SENSOR_LIST_ANCHOR_X, SENSOR_LIST_ANCHOR_Y+i);

            char sensor_index_str[5] = {0};

            c_attr(SENSOR_COLORS[sensor_group]);
            w_putc(&sensor_win, sensor_group+'A');
            ui2a(sensor_index, 10, sensor_index_str); 
            w_puts(&sensor_win, sensor_index_str);
            c_attr_reset();

        } 
    }
}

void
renderTask()
{
    RegisterAs(RENDERER_ADDRESS);

    term_init();

    // CONSOLE
    const usize CONSOLE_ANCHOR_X = 1;
    const usize CONSOLE_ANCHOR_Y = 29;
    const usize CONSOLE_MAX_LINES = 29;
    const usize CONSOLE_INNER_WIDTH = 58;
    Arena console_arena = arena_new(CONSOLE_MAX_LINES*(CONSOLE_INNER_WIDTH+1));
    CBuf* console_lines = cbuf_new(CONSOLE_MAX_LINES);
    Window console_win = win_init(2, 2, 60, 31);
    win_draw(&console_win);
    w_puts_mv(&console_win, "[console]", 2, 0);

    // PROMPT
    const usize PROMPT_ANCHOR_X = 3;
    const usize PROMPT_ANCHOR_Y = 1;
    usize prompt_length = 0;
    Window prompt_win = win_init(2, 33, 60, 3);
    win_draw(&prompt_win);
    w_putc_mv(&prompt_win, '>', 1, 1);

    // DIAGNOSTICS
    const usize DIAGNOSTIC_ANCHOR_X = 1;
    const usize DIAGNOSTIC_ANCHOR_Y = 1;
    Window diagnostic_win = win_init(63, 2, 20, 4);
    win_draw(&diagnostic_win);
    w_puts_mv(&diagnostic_win, "[diagnostics]", 2, 0);

    // SWITCH
    const usize SWITCH_ANCHOR_X = 1;
    const usize SWITCH_ANCHOR_Y = 1;
    Window switch_win = win_init(63, 23, 20, 13);
    win_draw(&switch_win);
    w_puts_mv(&switch_win, "[switches]", 2, 0);
    w_puts_mv(&switch_win, "01 .     12 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+0);
    w_puts_mv(&switch_win, "02 .     13 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+1);
    w_puts_mv(&switch_win, "03 .     14 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+2);
    w_puts_mv(&switch_win, "04 .     15 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+3);
    w_puts_mv(&switch_win, "05 .     16 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+4);
    w_puts_mv(&switch_win, "06 .     17 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+5);
    w_puts_mv(&switch_win, "07 .     18 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+6);
    w_puts_mv(&switch_win, "08 .    153 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+7);
    w_puts_mv(&switch_win, "09 .    154 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+8);
    w_puts_mv(&switch_win, "10 .    155 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+9);
    w_puts_mv(&switch_win, "11 .    156 .", SWITCH_ANCHOR_X, SWITCH_ANCHOR_Y+10);

    Create(3, &renderSensorWinTask, "Render sensor win");

    RendererMsg msg_buf;
    RendererResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(RendererMsg));

        if (msg_buf.type == RENDERER_APPEND_CONSOLE) {

            if (cbuf_len(console_lines) >= CONSOLE_MAX_LINES) {
                cbuf_pop_front(console_lines);
            }

            // need to first copy the data
            char* new_str = cstr_copy(&console_arena, msg_buf.data.append_console.line);
            cbuf_push_back(console_lines, new_str);

            for (usize i = 0; i < cbuf_len(console_lines); ++i) {

                // clear line first
                w_mv(&console_win, CONSOLE_ANCHOR_X, CONSOLE_ANCHOR_Y-cbuf_len(console_lines)+i+1);
                for (usize j = 0; j < CONSOLE_INNER_WIDTH; ++j) w_putc(&console_win, ' '); 

                // render the line
                w_mv(&console_win, CONSOLE_ANCHOR_X, CONSOLE_ANCHOR_Y-cbuf_len(console_lines)+i+1);
                w_puts(&console_win, (char*)cbuf_get(console_lines, i));

            }

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }
        else if (msg_buf.type == RENDERER_PROMPT) {

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
        else if (msg_buf.type == RENDERER_FLIP_SWITCH) {

            usize switch_id = msg_buf.data.flip_switch.switch_id;
            SwitchMode mode = msg_buf.data.flip_switch.mode;

            usize grid_id;
            if (1 <= switch_id && switch_id <= 18) {
                grid_id = switch_id - 1;
            }
            else if (153 <= switch_id && switch_id <= 156) {
                grid_id = switch_id - 153 + 18;
            }

            usize row = grid_id % 11;
            usize column = grid_id / 11;

            if (mode == SWITCH_MODE_CURVED) {
                c_attr(ATTR_YELLOW);
                w_putc_mv(&switch_win, 'C', SWITCH_ANCHOR_X+3+column*9, SWITCH_ANCHOR_Y+row);
            } else if (mode == SWITCH_MODE_STRAIGHT) {
                c_attr(ATTR_CYAN);
                w_putc_mv(&switch_win, 'S', SWITCH_ANCHOR_X+3+column*9, SWITCH_ANCHOR_Y+row);
            }
            c_attr_reset();

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }
        else if (msg_buf.type == RENDERER_DIAGNOSTIC) {
            char ticks_str[20] = {0};
            char idle_str[20] = {0};
            ui2a(msg_buf.data.diagnostic.ticks, 10, ticks_str);
            ui2a(msg_buf.data.diagnostic.idle_percent, 10, idle_str);

            // TODO don't need to keep reredendering this
            w_puts_mv(&diagnostic_win, "Tick:     ", DIAGNOSTIC_ANCHOR_X, DIAGNOSTIC_ANCHOR_Y);
            w_puts_mv(&diagnostic_win, "Idle:     ", DIAGNOSTIC_ANCHOR_X, DIAGNOSTIC_ANCHOR_Y+1);
            w_puts_mv(&diagnostic_win, ticks_str, DIAGNOSTIC_ANCHOR_X+6, DIAGNOSTIC_ANCHOR_Y);
            w_puts_mv(&diagnostic_win, idle_str, DIAGNOSTIC_ANCHOR_X+6, DIAGNOSTIC_ANCHOR_Y+1);
            w_putc(&diagnostic_win, '%');
            
            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }


    }
}
