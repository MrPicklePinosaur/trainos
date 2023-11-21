#include <trainterm.h>
#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include <ctype.h>
#include "render.h"
#include "prompt.h"
#include "user/path/track_data.h"
#include "user/path/train_data.h"
#include "user/sensor.h"
#include "user/switch.h"
#include "user/switch.h"
#include "user/trainstate.h"
#include "user/path/reserve.h"

#include "kern/dev/uart.h"
#include "kern/perf.h"

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
        struct {
            usize switch_id;
            SwitchMode mode;
        } flip_switch;
    } data;
} RendererMsg;

typedef struct { } RendererResp;

const Attr SENSOR_COLORS[5] = {ATTR_RED, ATTR_YELLOW, ATTR_GREEN, ATTR_CYAN, ATTR_MAGENTA};

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
renderZoneWinTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Delay(clock_server, 40);

    const usize ZONE_ANCHOR_X = 6;
    const usize ZONE_ANCHOR_Y = 1;
    const usize ZONE_COL_SPACING = 9;
    Window zone_win = win_init(84, 19, 38, 17);
    win_draw(&zone_win);
    w_puts_mv(&zone_win, "[zones]", 2, 0);
    w_puts_mv(&zone_win, "[00]     [13]     ", 1, 1);
    w_puts_mv(&zone_win, "[01]     [14]     ", 1, 2);
    w_puts_mv(&zone_win, "[02]     [15]     ", 1, 3);
    w_puts_mv(&zone_win, "[03]     [16]     ", 1, 4);
    w_puts_mv(&zone_win, "[04]     [17]     ", 1, 5);
    w_puts_mv(&zone_win, "[05]     [18]     ", 1, 6);
    w_puts_mv(&zone_win, "[06]     [19]     ", 1, 7);
    w_puts_mv(&zone_win, "[07]     [20]     ", 1, 8);
    w_puts_mv(&zone_win, "[08]     [21]     ", 1, 9);
    w_puts_mv(&zone_win, "[09]     [22]     ", 1, 10);
    w_puts_mv(&zone_win, "[10]     [23]     ", 1, 11);
    w_puts_mv(&zone_win, "[11]     [24]     ", 1, 12);
    w_puts_mv(&zone_win, "[12]     [25]     ", 1, 13);
    w_flush(&zone_win);

    Track* track = get_track_a();
    usize ticks = Time(clock_server);
    Arena tmp_base = arena_new(64);
    for (;;) {
        Arena tmp = tmp_base;

        ticks += 100; // update every 1 second
        DelayUntil(clock_server, ticks); 

        // update all zone states (this is kinda inefficient, maybe do a listener model)
        usize* reservation = zone_dump();
        for (usize i = 0; i < track->zone_count; ++i) {
            usize res = reservation[i];
            char* res_str = cstr_format(&tmp, "%d", res);
            usize x = ZONE_ANCHOR_X + ((i/13 == 0) ? 0 : ZONE_COL_SPACING);
            w_puts_mv(&zone_win, "    ", x, ZONE_ANCHOR_Y+(i%13));
            w_puts_mv(&zone_win, res_str, x, ZONE_ANCHOR_Y+(i%13));
            w_flush(&zone_win);
        }
    }

    Exit();
}


void
renderTrainStateWinTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    const int TRAIN_STATE_TABLE_Y = 3;
    const int TRAIN_STATE_TABLE_CURR_X = 8;
    const int TRAIN_STATE_TABLE_NEXT_X = 14;
    const int TRAIN_STATE_TABLE_ZONE_X = 20;
    const int TRAIN_STATE_TABLE_SPD_X = 26;

    Delay(clock_server, 30);

    Track* track = get_track_a();

    Window train_state_win = win_init(84, 2, 38, 17);
    win_draw(&train_state_win);
    w_puts_mv(&train_state_win, "[train state]", 2, 0);

    w_puts_mv(&train_state_win, "train  curr  next  zone  spd   dest", 1, 2);
    w_puts_mv(&train_state_win, "2                                  ", 1, 3);
    w_puts_mv(&train_state_win, "47                                 ", 1, 4);
    w_puts_mv(&train_state_win, "58                                 ", 1, 5);
    w_puts_mv(&train_state_win, "77                                 ", 1, 6);

    w_flush(&train_state_win);

    Arena tmp_base = arena_new(256);
    for (;;) {
        Arena tmp = tmp_base;
        
        Pair_usize_usize res = TrainstateWaitForSensor(trainstate_server, -1);
        usize train = res.first;
        usize new_pos = res.second;
        str8 sensor_name = sensor_id_to_name(new_pos, &tmp);

        TrainState state = TrainstateGet(trainstate_server, train);
        usize speed = state.speed;
        usize next_pos = track_next_node(switch_server, track, &track->nodes[new_pos]);
        str8 next_sensor_name = sensor_id_to_name(next_pos, &tmp);
        ZoneId zone = track->nodes[new_pos].reverse->zone;
        str8 dest_sensor_name = sensor_id_to_name(state.dest, &tmp);

        w_puts_mv(&train_state_win, "     ", TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y+get_train_index(train));
        w_puts_mv(&train_state_win, str8_to_cstr(sensor_name), TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y+get_train_index(train));
        w_puts_mv(&train_state_win, str8_to_cstr(next_sensor_name), TRAIN_STATE_TABLE_CURR_X+6, TRAIN_STATE_TABLE_Y+get_train_index(train));
        w_puts_mv(&train_state_win, cstr_format(&tmp, "%d", zone), TRAIN_STATE_TABLE_CURR_X+12, TRAIN_STATE_TABLE_Y+get_train_index(train));
        w_puts_mv(&train_state_win, cstr_format(&tmp, "%d", speed), TRAIN_STATE_TABLE_CURR_X+18, TRAIN_STATE_TABLE_Y+get_train_index(train));
        w_puts_mv(&train_state_win, str8_to_cstr(dest_sensor_name), TRAIN_STATE_TABLE_CURR_X+24, TRAIN_STATE_TABLE_Y+get_train_index(train));
        w_flush(&train_state_win);
    }
    Exit();
}

void
renderSensorWinTask()
{
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);

    const usize SENSOR_LIST_ANCHOR_X = 1;
    const usize SENSOR_LIST_ANCHOR_Y = 2;
    const usize MAX_SENSORS = 14;
    CBuf* triggered_sensors = cbuf_new(MAX_SENSORS);
    Window sensor_win = win_init(63, 6, 20, 17);
    win_draw(&sensor_win);
    w_puts_mv(&sensor_win, "[sensors]", 2, 0);

    for (;;) {

        w_flush(&sensor_win);

        usize next_sensor_id = WaitForSensor(sensor_server, -1);
        
        if (cbuf_len(triggered_sensors) >= MAX_SENSORS) {
            cbuf_pop_back(triggered_sensors);
        }
        cbuf_push_front(triggered_sensors, (void*)next_sensor_id);

        for (usize i = 0; i < min(MAX_SENSORS, cbuf_len(triggered_sensors)); ++i) {
            // build string from raw sensor id
            usize sensor_id = (usize)cbuf_get(triggered_sensors, i);

            usize sensor_group = sensor_id / 16;
            usize sensor_index = (sensor_id % 16) + 1;
            w_mv(&sensor_win, SENSOR_LIST_ANCHOR_X, SENSOR_LIST_ANCHOR_Y+i);
            w_puts(&sensor_win, "   ");
            w_mv(&sensor_win, SENSOR_LIST_ANCHOR_X, SENSOR_LIST_ANCHOR_Y+i);

            char sensor_index_str[5] = {0};

            w_attr(&sensor_win, SENSOR_COLORS[sensor_group]);
            w_putc(&sensor_win, sensor_group+'A');
            ui2a(sensor_index, 10, sensor_index_str); 
            w_puts(&sensor_win, sensor_index_str);
            w_attr_reset(&sensor_win);

        } 
    }
}


void
renderSwitchWinTask()
{
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    // Arbritrary delay to prevent context switch issues during draw calls
    // TODO get rid of this SOON
    Delay(clock_server, 20);

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

    for (;;) {
        w_flush(&switch_win);

        WaitForSwitchResult data = WaitForSwitch(switch_server, -1);
        isize switch_id = data.first;
        SwitchMode mode = data.second;

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
            w_attr(&switch_win, ATTR_YELLOW);
            w_putc_mv(&switch_win, 'C', SWITCH_ANCHOR_X+3+column*9, SWITCH_ANCHOR_Y+row);
        } else if (mode == SWITCH_MODE_STRAIGHT) {
            w_attr(&switch_win, ATTR_CYAN);
            w_putc_mv(&switch_win, 'S', SWITCH_ANCHOR_X+3+column*9, SWITCH_ANCHOR_Y+row);
        }
        w_attr_reset(&switch_win);
    }
}


void
renderDiagnosticWinTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    // TODO add arbritrary delay to prevent context switching during draw calls
    // TODO get rid of this SOON
    Delay(clock_server, 10);

    const usize DIAGNOSTIC_ANCHOR_X = 1;
    const usize DIAGNOSTIC_ANCHOR_Y = 1;
    Window diagnostic_win = win_init(63, 2, 20, 4);
    win_draw(&diagnostic_win);
    w_puts_mv(&diagnostic_win, "[diagnostics]", 2, 0);

    Arena tmp_base = arena_new(64);
    usize ticks = Time(clock_server);
    for (;;) {
        Arena tmp = tmp_base;

        w_flush(&diagnostic_win);

        ticks += 10; // update every 1/10 second
        DelayUntil(clock_server, ticks); 

        char idle_str[20] = {0};

        ui2a(get_idle_time(), 10, idle_str);

        u32 f_min = (ticks / 100) / 60;
        u32 f_secs = (ticks / 100) % 60;
        u32 f_tenths = (ticks % 100);

        char* time_fmt = cstr_format(&tmp, "%d:%d:%d", f_min, f_secs, f_tenths);

        // TODO don't need to keep reredendering this
        w_puts_mv(&diagnostic_win, "Time:            ", DIAGNOSTIC_ANCHOR_X, DIAGNOSTIC_ANCHOR_Y);
        w_puts_mv(&diagnostic_win, "Idle:            ", DIAGNOSTIC_ANCHOR_X, DIAGNOSTIC_ANCHOR_Y+1);
        w_puts_mv(&diagnostic_win, time_fmt, DIAGNOSTIC_ANCHOR_X+6, DIAGNOSTIC_ANCHOR_Y);
        w_puts_mv(&diagnostic_win, idle_str, DIAGNOSTIC_ANCHOR_X+6, DIAGNOSTIC_ANCHOR_Y+1);
        w_puts(&diagnostic_win, "%%");
    }

    Exit();
}

void
renderTask()
{
    RegisterAs(RENDERER_ADDRESS);

    term_init();

    set_log_mode(LOG_MODE_TRAIN_TERM);

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
    w_flush(&console_win);

    // PROMPT
    const usize PROMPT_ANCHOR_X = 3;
    const usize PROMPT_ANCHOR_Y = 1;
    const usize PROMPT_MAX_LEN = 56;
    usize prompt_length = 0;
    Window prompt_win = win_init(2, 33, 60, 3);
    win_draw(&prompt_win);
    w_putc_mv(&prompt_win, '>', 1, 1);

    // draw the cursor first
    w_attr(&prompt_win, ATTR_BLINK);
    w_puts_mv(&prompt_win, "█", PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
    w_attr_reset(&prompt_win);

    w_flush(&prompt_win);

    Create(5, &renderSwitchWinTask, "Render Switch Window");
    Create(5, &renderSensorWinTask, "Render Sensor Window");
    Create(5, &renderDiagnosticWinTask, "Render Diagnostic Window");
    Create(5, &renderTrainStateWinTask, "Render Train State Window");
    /* Create(5, &renderZoneWinTask, "Render Zone Window"); */

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
                w_flush(&console_win);
            }

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }
        else if (msg_buf.type == RENDERER_PROMPT) {

            char ch = msg_buf.data.prompt.ch;
            if (ch == CH_BACKSPACE) {
                prompt_length = usize_sub(prompt_length, PROMPT_ANCHOR_Y);
                w_puts_mv(&prompt_win, "  ", PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
            }
            else if (ch == CH_ENTER) {
                for (usize i = 0; i < prompt_length+1; ++i) w_putc_mv(&prompt_win, ' ', PROMPT_ANCHOR_X+i, PROMPT_ANCHOR_Y);
                prompt_length = 0;
            }
            else if ((isalnum(ch) || isblank(ch) || isprint(ch)) && prompt_length < PROMPT_MAX_LEN) {
                // normal character
                w_putc_mv(&prompt_win, ch, PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
                prompt_length += 1;
            }
            
            // draw the cursor
            w_attr(&prompt_win, ATTR_BLINK);
            w_puts_mv(&prompt_win, "█", PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
            w_attr_reset(&prompt_win);
            w_flush(&prompt_win);

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }
    }
}


// soley responsible for rendering the ui
void
uiTask()
{
    set_log_mode(LOG_MODE_TRAIN_TERM);

    Tid render_tid = Create(5, &renderTask, "Render Task");
    Tid prompt_tid = Create(5, &promptTask, "Prompt Task");

    WaitTid(prompt_tid);

    Exit();
}
