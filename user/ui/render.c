#include <trainterm.h>
#include <trainstd.h>
#include <trainsys.h>
#include <ctype.h>
#include "render.h"
#include "prompt.h"
#include "user/path/track_data.h"
#include "user/nameserver.h"
#include "user/sensor.h"
#include "user/switch.h"
#include "user/clock.h"
#include "user/switch.h"

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

    const int TRAIN_STATE_TABLE_Y = 3;
    const int TRAIN_STATE_TABLE_CURR_X = 8;
    const int TRAIN_STATE_TABLE_NEXT_X = 14;
    const int TRAIN_STATE_TABLE_TERR_X = 20;
    const int TRAIN_STATE_TABLE_DERR_X = 26;

    Delay(clock_server, 30);

    // TODO i dont like how we are loading the track twice (perhaps have some global accessable way of querying current track)
    Arena arena = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Track track = track_a_init(&arena);

    Window train_state_win = win_init(84, 2, 32, 17);
    win_draw(&train_state_win);
    w_puts_mv(&train_state_win, "[train state]", 2, 0);

    w_puts_mv(&train_state_win, "train  curr  next  terr  derr", 1, 2);
    w_puts_mv(&train_state_win, "2                            ", 1, 3);
    w_puts_mv(&train_state_win, "47                           ", 1, 4);

    Arena tmp_base = arena_new(256);

    const usize TRAIN = 2;
    usize next_sensor_id = 0;
    usize last_sensor_time = 0;
    usize predicted_sensor_time = 0;

    for (;;) {
        Arena tmp = tmp_base; 

        // TODO this currently only supports one train
        int sensor_id = WaitForSensor(sensor_server, -1);

        // TODO this is bad duplicated code (should use cstr_format instead)
        usize sensor_group = sensor_id / 16;
        usize sensor_index = (sensor_id % 16) + 1;
        c_attr(SENSOR_COLORS[sensor_group]);
        char* sensor_str = cstr_format(&tmp_base, "%c%d", sensor_group+'A', sensor_index);
        w_puts_mv(&train_state_win, "    ", TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y);
        w_puts_mv(&train_state_win, sensor_str, TRAIN_STATE_TABLE_CURR_X, TRAIN_STATE_TABLE_Y);
        c_attr_reset();

        // predict what the next sensor will be using switch states to walk the graph
        usize cur_node_ind = (usize)map_get(&track.map, str8_from_cstr(sensor_str), &arena);
        TrackNode* cur_node = &track.nodes[cur_node_ind];
        ULOG_INFO("starting at %s", cur_node->name);
        do {
            
            ULOG_INFO("walking to %s", cur_node->name);

            if (cur_node->type == NODE_BRANCH) {
                // query switch state to find next
                SwitchMode switch_mode = SwitchQuery(switch_server, cur_node->num);
                if (switch_mode == SWITCH_MODE_UNKNOWN) {
                    cur_node = NULL;
                    break;
                }
                else if (switch_mode == SWITCH_MODE_STRAIGHT) {
                    cur_node = cur_node->edge[DIR_STRAIGHT].dest;
                }
                else if (switch_mode == SWITCH_MODE_CURVED) {
                    cur_node = cur_node->edge[DIR_CURVED].dest;
                }
            } else {
                cur_node = cur_node->edge[DIR_AHEAD].dest;
            }

        } while (cur_node->type != NODE_SENSOR);

        if (cur_node == NULL) {
            w_puts_mv(&train_state_win, "XXXX", TRAIN_STATE_TABLE_NEXT_X, TRAIN_STATE_TABLE_Y);
        } else {
            c_attr(SENSOR_COLORS[cur_node->name[0]-'A']);
            w_puts_mv(&train_state_win, cur_node->name, TRAIN_STATE_TABLE_NEXT_X, TRAIN_STATE_TABLE_Y);
            c_attr_reset();
        }

         
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
            c_attr(ATTR_YELLOW);
            w_putc_mv(&switch_win, 'C', SWITCH_ANCHOR_X+3+column*9, SWITCH_ANCHOR_Y+row);
        } else if (mode == SWITCH_MODE_STRAIGHT) {
            c_attr(ATTR_CYAN);
            w_putc_mv(&switch_win, 'S', SWITCH_ANCHOR_X+3+column*9, SWITCH_ANCHOR_Y+row);
        }
        c_attr_reset();
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

    Create(3, &renderSwitchWinTask, "Render switch win");
    Create(3, &renderSensorWinTask, "Render sensor win");
    Create(3, &renderDiagnosticWinTask, "Render diagnostic win");
    Create(3, &renderTrainStateWinTask, "Render train state win");

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
            else if (isalnum(ch) || isblank(ch) || isprint(ch)) {
                // normal character
                // TODO max length for prompt
                w_putc_mv(&prompt_win, ch, PROMPT_ANCHOR_X+prompt_length, PROMPT_ANCHOR_Y);
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

    Tid render_tid = Create(3, &renderTask, "render task");
    Tid prompt_tid = Create(2, &promptTask, "prompt task");

    WaitTid(prompt_tid);

    //  TODO impleement Kill() syscall

    Exit();
}
