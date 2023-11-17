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
#include "user/trainpos.h"

#include "kern/dev/uart.h"

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
renderTrainStateWinTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid trainpos_server = WhoIs(TRAINPOS_ADDRESS);

    const int TRAIN_STATE_TABLE_Y = 3;
    const int TRAIN_STATE_TABLE_CURR_X = 8;
    const int TRAIN_STATE_TABLE_NEXT_X = 14;
    const int TRAIN_STATE_TABLE_TERR_X = 20;
    const int TRAIN_STATE_TABLE_DERR_X = 26;

    Delay(clock_server, 30);

    // TODO i dont like how we are loading the track twice (perhaps have some global accessable way of querying current track)
    Track* track = get_track_a();

    Window train_state_win = win_init(84, 2, 32, 17);
    win_draw(&train_state_win);
    w_puts_mv(&train_state_win, "[train state]", 2, 0);

    w_puts_mv(&train_state_win, "train  curr  next  terr  derr", 1, 2);
    w_puts_mv(&train_state_win, "2                            ", 1, 3);
    w_puts_mv(&train_state_win, "47                           ", 1, 4);
    w_puts_mv(&train_state_win, "58                           ", 1, 5);
    w_puts_mv(&train_state_win, "77                           ", 1, 6);

    w_flush(&train_state_win);

    Arena tmp_base = arena_new(256);
    for (;;) {
        Arena tmp = tmp_base;
        TrainPosWaitResult res = trainPosWait(trainpos_server, -1);
        str8 sensor_name = sensor_id_to_name(res.pos, &tmp);
        w_puts_mv(&train_state_win, "     ", TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y+get_train_index(res.train));
        w_puts_mv(&train_state_win, str8_to_cstr(sensor_name), TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y+get_train_index(res.train));
        w_flush(&train_state_win);
    }
    // TODO a lot of this code is horrible and needs a rewrite
#if 0
    Arena arena = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Arena tmp_base = arena_new(256);
    Arena tmp;

    const usize TRAIN = 2;
    usize next_sensor_id = 0;
    usize last_sensor_time = 0;
    usize predicted_sensor_time = 0;

    for (;;) {
        tmp = tmp_base; 
        w_flush(&train_state_win);

        // TODO this currently only supports one train
        usize sensor_id = WaitForSensor(sensor_server, -1);
        Delay(clock_server, 5);

        // TODO this is bad duplicated code (should use cstr_format instead)
        usize sensor_group = sensor_id / 16;
        usize sensor_index = (sensor_id % 16) + 1;
        w_attr(&train_state_win, SENSOR_COLORS[sensor_group]);
        str8 sensor_str = str8_format(&tmp, "%c%d", sensor_group+'A', sensor_index);
        w_puts_mv(&train_state_win, "    ", TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y);
        w_puts_mv(&train_state_win, str8_to_cstr(sensor_str), TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y);
        w_attr_reset(&train_state_win);

        // predict what the next sensor will be using switch states to walk the graph

        TrackNode cur_node = *track_node_by_sensor_id(track, sensor_id);
        bool is_unknown = false;
        int dist_to_next = 0; // distance from current sensor to next sensor
        //ULOG_INFO("starting at %s", cur_node.name);
        do {

            if (cur_node.type == NODE_EXIT || cur_node.type == NODE_NONE) {
                //ULOG_INFO("hit exit or none node");
                is_unknown = true;
                break;
            }

            if (cur_node.type == NODE_BRANCH) {
                // query switch state to find next
                SwitchMode switch_mode = SwitchQuery(switch_server, cur_node.num);
                //ULOG_INFO("queried switch mode for switch %d: %d", cur_node.num, switch_mode);
                if (switch_mode == SWITCH_MODE_UNKNOWN) {
                    //ULOG_INFO("hit switch with unknown state");
                    is_unknown = true;
                    break;
                }
                else if (switch_mode == SWITCH_MODE_STRAIGHT) {
                    cur_node = *(cur_node.edge[DIR_STRAIGHT].dest);
                    dist_to_next += cur_node.edge[DIR_STRAIGHT].dist;
                }
                else if (switch_mode == SWITCH_MODE_CURVED) {
                    cur_node = *(cur_node.edge[DIR_CURVED].dest);
                    dist_to_next += cur_node.edge[DIR_CURVED].dist;
                } else {
                    UNREACHABLE("unexpected switch mode");
                }
            } else {
                cur_node = *(cur_node.edge[DIR_AHEAD].dest);
                dist_to_next += cur_node.edge[DIR_AHEAD].dist;
            }

            //ULOG_INFO("walking to %s", cur_node.name);

        } while (cur_node.type != NODE_SENSOR);

        if (is_unknown) {
            w_puts_mv(&train_state_win, "XXXXX", TRAIN_STATE_TABLE_NEXT_X, TRAIN_STATE_TABLE_Y);
            continue;
        }

        // print next sensor
        usize color_index = cur_node.name[0]-'A';
        if (!(0 <= color_index && color_index <= 5)) {
            PANIC("INVALID COLOR INDEX %d", color_index);
        }
        w_attr(&train_state_win, SENSOR_COLORS[color_index]);
        w_puts_mv(&train_state_win, "     ", TRAIN_STATE_TABLE_NEXT_X, TRAIN_STATE_TABLE_Y);
        w_puts_mv(&train_state_win, cur_node.name, TRAIN_STATE_TABLE_NEXT_X, TRAIN_STATE_TABLE_Y);
        w_attr_reset(&train_state_win);

        usize train_speed = TrainstateGet(trainstate_server, TRAIN) & TRAIN_SPEED_MASK;
        if (!(train_speed == TRAIN_SPEED_SNAIL || train_speed == TRAIN_SPEED_LOW || train_speed == TRAIN_SPEED_MED || train_speed == TRAIN_SPEED_HIGH)) {
            continue;
        }

        // TODO we only support prediction for speed 5, 8, 11, 14
        // get train speed
        usize train_vel = train_data_vel(TRAIN, train_speed);

        isize elapsed = Time(clock_server) - last_sensor_time; // elapsed is in ticks
        last_sensor_time = Time(clock_server);

        // Compare elapsed time with our predicted
        if (predicted_sensor_time != 0) {
            isize t_err = elapsed-predicted_sensor_time;
            w_puts_mv(&train_state_win, "     ", TRAIN_STATE_TABLE_TERR_X, TRAIN_STATE_TABLE_Y);
            //w_puts_mv(&train_state_win, cstr_format(&tmp, "%d", t_err), TRAIN_STATE_TABLE_TERR_X, TRAIN_STATE_TABLE_Y);
            w_mv(&train_state_win, TRAIN_STATE_TABLE_TERR_X, TRAIN_STATE_TABLE_Y);
            uart_printf(CONSOLE, "%d", t_err);
            isize d_err = (t_err)*train_vel/100;
            // TODO there is some really weird memory bug somewhere here, should run some extensives tests on cstr_format
            //char* d_err_str = cstr_format(&tmp, "%d", d_err);
            // TODO don't print if too long :P
            //if (strlen(d_err_str) <= 5) {
            //    w_puts_mv(&train_state_win, d_err_str, TRAIN_STATE_TABLE_DERR_X, TRAIN_STATE_TABLE_Y);
            //}
            w_puts_mv(&train_state_win, "     ", TRAIN_STATE_TABLE_DERR_X, TRAIN_STATE_TABLE_Y);
            if (-9999 <= d_err && d_err <= 9999) {
                w_mv(&train_state_win, TRAIN_STATE_TABLE_DERR_X, TRAIN_STATE_TABLE_Y);
                uart_printf(CONSOLE, "%d", d_err);
            }
        } else {
            w_puts_mv(&train_state_win, "XXXXX", TRAIN_STATE_TABLE_TERR_X, TRAIN_STATE_TABLE_Y);
            w_puts_mv(&train_state_win, "XXXXX", TRAIN_STATE_TABLE_DERR_X, TRAIN_STATE_TABLE_Y);
        }

        // predict sensor time
        // TODO careful for division by zero
        if (train_vel == 0) {
            PANIC("division by zero");
        }
        predicted_sensor_time = (dist_to_next/train_vel)*100; // predicted is in ticks
    }
#endif
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

    usize ticks = Time(clock_server);
    for (;;) {

        w_flush(&diagnostic_win);

        ticks += 100; // update every second
        DelayUntil(clock_server, ticks); 

        char ticks_str[20] = {0};
        char idle_str[20] = {0};

        ui2a(ticks, 10, ticks_str);
        ui2a(get_idle_time(), 10, idle_str);

        // TODO don't need to keep reredendering this
        w_puts_mv(&diagnostic_win, "Tick:     ", DIAGNOSTIC_ANCHOR_X, DIAGNOSTIC_ANCHOR_Y);
        w_puts_mv(&diagnostic_win, "Idle:     ", DIAGNOSTIC_ANCHOR_X, DIAGNOSTIC_ANCHOR_Y+1);
        w_puts_mv(&diagnostic_win, ticks_str, DIAGNOSTIC_ANCHOR_X+6, DIAGNOSTIC_ANCHOR_Y);
        w_puts_mv(&diagnostic_win, idle_str, DIAGNOSTIC_ANCHOR_X+6, DIAGNOSTIC_ANCHOR_Y+1);
        w_putc(&diagnostic_win, '%');
        
    }

    Exit();
}

void
renderDebugConsoleTask()
{
    RegisterAs(DEBUG_ADDRESS);

    const usize DEBUG_ANCHOR_X = 1;
    const usize DEBUG_ANCHOR_Y = 1;
    const usize DEBUG_MAX_LINES = 32;
    const usize DEBUG_INNER_WIDTH = 58;
    Arena debug_arena = arena_new(20*DEBUG_MAX_LINES*(DEBUG_INNER_WIDTH+1));
    CBuf* debug_lines = cbuf_new(DEBUG_MAX_LINES);
    Window debug_win = win_init(117, 2, 60, 34);
    win_draw(&debug_win);
    w_puts_mv(&debug_win, "[debug]", 2, 0);
    w_flush(&debug_win);

    char* msg_buf;
    struct {} reply_buf; // dummy
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(char*));
        if (msg_len < 0) {
            ULOG_WARN("[DEBUG CONSOLE SERVER] Error when receiving");
            continue;
        }

        // copy message
        char* copied = cstr_copy(&debug_arena, msg_buf);
        Reply(from_tid, (char*)&reply_buf, 0);

        // scroll terminal if needed
        if (cbuf_len(debug_lines) >= DEBUG_MAX_LINES) {
            cbuf_pop_front(debug_lines);
        }

        cbuf_push_back(debug_lines, copied);

        // rerender window
        for (usize i = 0; i < DEBUG_MAX_LINES; ++i) {
            w_mv(&debug_win, DEBUG_ANCHOR_X, DEBUG_ANCHOR_Y+i);
            for (usize j = 0; j < DEBUG_INNER_WIDTH; ++j) w_putc(&debug_win, ' ');
            
            w_mv(&debug_win, DEBUG_ANCHOR_X, DEBUG_ANCHOR_Y+i);
            w_puts(&debug_win, cbuf_get(debug_lines, i));
        }
        w_flush(&debug_win);

    }

    Exit();
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
    w_flush(&console_win);

    // PROMPT
    const usize PROMPT_ANCHOR_X = 3;
    const usize PROMPT_ANCHOR_Y = 1;
    usize prompt_length = 0;
    Window prompt_win = win_init(2, 33, 60, 3);
    win_draw(&prompt_win);
    w_putc_mv(&prompt_win, '>', 1, 1);
    w_flush(&prompt_win);

    Tid debug_console_server = Create(3, &renderDebugConsoleTask, "Render Debug Console Window");
    set_log_server(debug_console_server);
    set_log_mode(LOG_MODE_TRAIN_TERM);

    Create(3, &renderSwitchWinTask, "Render Switch Window");
    Create(3, &renderSensorWinTask, "Render Sensor Window");
    Create(3, &renderDiagnosticWinTask, "Render Diagnostic Window");
    Create(3, &renderTrainStateWinTask, "Render Train State Window");

    for (usize i = 0; i < 10; ++i) {
        ULOG_DEBUG("render init complete, %d", i);
    }

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
                w_putc_mv(&prompt_win, ' ', PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
                w_flush(&prompt_win);
            }
            else if (ch == CH_ENTER) {
                for (usize i = 0; i < prompt_length; ++i) w_putc_mv(&prompt_win, ' ', PROMPT_ANCHOR_X+i, PROMPT_ANCHOR_Y);
                w_flush(&prompt_win);
                prompt_length = 0;
            }
            else if (isalnum(ch) || isblank(ch) || isprint(ch)) {
                // normal character
                // TODO max length for prompt
                w_putc_mv(&prompt_win, ch, PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
                w_flush(&prompt_win);
                prompt_length += 1;
            }

            Reply(from_tid, (char*)&reply_buf, sizeof(RendererResp));
        }
    }
}


// soley responsible for rendering the ui
void
uiTask()
{

    Tid render_tid = Create(3, &renderTask, "Render Task");
    Tid prompt_tid = Create(2, &promptTask, "Prompt Task");

    WaitTid(prompt_tid);

    //  TODO impleement Kill() syscall

    Exit();
}
