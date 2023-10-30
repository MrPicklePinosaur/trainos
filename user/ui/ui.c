#include <trainstd.h>
#include <trainsys.h>
#include <trainterm.h>
#include <ctype.h>
#include "user/clock.h"
#include "user/io.h"
#include "user/nameserver.h"
#include "parser.h"
#include "ui.h"
#include "render.h"

#include "kern/perf.h"

#define NUMBER_OF_TRAINS 80
#define TRAIN_SPEED_MASK     0b01111
#define TRAIN_LIGHTS_MASK    0b10000

#define BYTE_COUNT 10
#define UNIT_COUNT 5

typedef u8 TrainState;

void executeCommand(Arena tmp, Tid marklin_server, Tid clock_server, Tid renderer_server, TrainState* train_state, ParserResult command);

// task for querying switch states
void
switchStateTask()
{
    u8 sensor_state[BYTE_COUNT] = {0};
    u8 prev_sensor_state[BYTE_COUNT] = {0};

    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid renderer_server = WhoIs(RENDERER_ADDRESS);

    for (;;) {
        marklin_dump_s88(marklin_server, UNIT_COUNT);

        for (usize i = 0; i < BYTE_COUNT; ++i) {
            u8 sensor_byte = Getc(marklin_server);
            prev_sensor_state[i] = sensor_state[i];
            sensor_state[i] = sensor_byte;
            u8 triggered = ~(prev_sensor_state[i]) & sensor_state[i];
            for (usize j = 0; j < 8; ++j) {
                if (((triggered >> j) & 0x1) == 1) {
                    u8 index = (7-j);
                    renderer_sensor_triggered(renderer_server, i*8+index);
                }
            }
        }

        // TODO maybe should use DelayUntil to guarentee uniform fetches
        Delay(clock_server, 20);
    }

    Exit();
}

// task for getting user input form the console
void
promptTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_CONSOLE);
    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid renderer_server = WhoIs(RENDERER_ADDRESS);

    TrainState train_state[NUMBER_OF_TRAINS] = {0};

    CBuf* line = cbuf_new(32);

    // TODO: by using a temp arena, we need to ensure that the console server reads the line we send it and copy the data before we write the next line. Otherwise we will end up with duplicate lines.
    Arena tmp_arena = arena_new(256);

    marklin_init(marklin_server);

    for (;;) {
        int c = Getc(io_server);

        renderer_prompt(renderer_server, c);

        if (isalnum(c) || isblank(c)) {
            cbuf_push_back(line, c);
        } else if (c == CH_ENTER) {
            // drain the buffer
            usize input_len = cbuf_len(line);
            char completed_line[input_len+1];
            for (u8 i = 0; i < input_len; ++i)
                completed_line[i] = (char)cbuf_get(line, i);

            completed_line[input_len] = 0;
            cbuf_clear(line);

            // it is okay to parse and execute commands synchronously here, since we don't want to print the next prompt line until the command finishes
            // TODO since we are using a tmp arena, we can technically 
            ParserResult parsed = parse_command(str8(completed_line));
            executeCommand(tmp_arena, marklin_server, clock_server, renderer_server, train_state, parsed);

        } else if (c == CH_BACKSPACE) {
            cbuf_pop_back(line);
        }

    }
}

void
executeCommand(Arena tmp, Tid marklin_server, Tid clock_server, Tid renderer_server, TrainState* train_state, ParserResult command)
{
    switch (command._type) {
        case PARSER_RESULT_TRAIN_SPEED: {
            uint32_t train = command._data.train_speed.train;
            uint32_t speed = command._data.train_speed.speed;
            train_state[train] = (train_state[train] & ~TRAIN_SPEED_MASK) | speed;

            char* msg = cstr_format(&tmp, "Setting train %d to speed %d", train, speed);
            renderer_append_console(renderer_server, msg);
            marklin_train_ctl(marklin_server, train, train_state[train]);

			break;
		}
        case PARSER_RESULT_REVERSE: {
            uint32_t train = command._data.reverse.train;

            char* msg = cstr_format(&tmp, "Reversing train %d", train);
            renderer_append_console(renderer_server, msg);

            marklin_train_ctl(marklin_server, train, SPEED_STOP);
            Delay(clock_server, 400); // TODO arbritrary delay
            marklin_train_ctl(marklin_server, train, SPEED_REVERSE);
            Delay(clock_server, 10); // TODO arbritrary delay
            marklin_train_ctl(marklin_server, train, train_state[train]);

			break;
		}
        case PARSER_RESULT_SWITCH: {
            u32 switch_id = command._data.switch_control.switch_id;
            SwitchMode switch_mode = command._data.switch_control.switch_mode;

            char* msg = cstr_format(&tmp, "Setting switch %x to %s", switch_id, (switch_mode == SWITCH_MODE_CURVED) ? "curved" : "straight");
            renderer_append_console(renderer_server, msg);

            marklin_switch_ctl(marklin_server, switch_id, switch_mode);
            renderer_flip_switch(renderer_server, switch_id, switch_mode);
			break;
		}
        case PARSER_RESULT_STOP: {
            renderer_append_console(renderer_server, "Powering off marklin");
            marklin_stop(marklin_server);
			break;
		}
        case PARSER_RESULT_GO: {
            renderer_append_console(renderer_server, "Powering on marklin");
            marklin_go(marklin_server);
			break;
		}
        case PARSER_RESULT_LIGHTS: {
            uint32_t train = command._data.lights.train;
            bool light_state = command._data.lights.state;

            char* msg = cstr_format(&tmp, "Turned lights on train %d %s", train, (light_state) ? "on" : "off");
            renderer_append_console(renderer_server, msg);

            if (light_state) {
                train_state[train] |= TRAIN_LIGHTS_MASK;
            } else {
                train_state[train] &= ~TRAIN_LIGHTS_MASK;
            }
            marklin_train_ctl(marklin_server, train, train_state[train]);
			break;
		}
        case PARSER_RESULT_QUIT: {
            // exit task and restore console state
            term_clear();
            Exit();
			break; // unreachable
		}
        default: {
            renderer_append_console(renderer_server, "Invalid command");
        }
    }
}

void
diagnosticTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid renderer_server = WhoIs(RENDERER_ADDRESS);
    usize ticks = Time(clock_server);
    for (;;) {
        ticks += 100; // update every second
        DelayUntil(clock_server, ticks); 
        renderer_diagnostic(renderer_server, ticks, get_idle_time());
    }

    Exit();
}

// soley responsible for rendering the ui
void
uiTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    Tid render_tid = Create(3, &renderTask, "render task");
    Tid diagnostic_tid = Create(3, &diagnosticTask, "diagnostic task");
    Tid prompt_tid = Create(2, &promptTask, "prompt task");
    Tid switch_state_tid = Create(2, &switchStateTask, "switch state task");

    WaitTid(prompt_tid);

    //  TODO impleement Kill() syscall

    Exit();
}
