#include <trainstd.h>
#include <trainsys.h>
#include <ctype.h>
#include "user/clock.h"
#include "user/io.h"
#include "user/nameserver.h"
#include "parser.h"
#include "ui.h"
#include "render.h"

#define NUMBER_OF_TRAINS 80
#define TRAIN_SPEED_MASK     0b01111
#define TRAIN_LIGHTS_MASK    0b10000

typedef u8 TrainState;

void executeCommand(Tid marklin_server, Tid clock_server, TrainState* train_state, ParserResult command);

// task for querying switch states
void
switchStateTask()
{
    const u32 switch_count = 5;
    u32 switch_state[switch_count];

    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    for (;;) {
        marklin_dump_s88(marklin_server, switch_count);

        for (u32 i = 0; i < switch_count; ++i) {
            switch_state[i] = Getc(marklin_server);
        }

        // TODO send to display server to render new switch state

        // TODO maybe should use DelayUntil to guarentee uniform fetches
        Delay(clock_server, 10);
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

    for (;;) {
        int c = Getc(io_server);

        renderer_prompt(renderer_server, c);

        if (isalnum(c) || isblank(c)) {
            cbuf_push_back(line, c);
        } else if (c == CH_ENTER) {
            // drain the buffer
            usize input_len = cbuf_len(line);
            char completed_line[input_len];
            for (u8 i = 0; i < input_len; ++i)
                completed_line[i] = cbuf_pop_front(line);

            // it is okay to parse and execute commands synchronously here, since we don't want to print the next prompt line until the command finishes
            ParserResult parsed = parse_command(str8(completed_line));
            if (parsed._type == PARSER_RESULT_ERROR) {
                // TODO print error message
                renderer_append_console(renderer_server, "invalid command");
                continue;
            }
            executeCommand(marklin_server, clock_server, train_state, parsed);
            renderer_append_console(renderer_server, "valid command");


        } else if (c == CH_BACKSPACE) {
            cbuf_pop_back(line);
        }
    }
}

void
executeCommand(Tid marklin_server, Tid clock_server, TrainState* train_state, ParserResult command)
{
    switch (command._type) {
        case PARSER_RESULT_TRAIN_SPEED: {
            uint32_t train = command._data.train_speed.train;
            uint32_t speed = command._data.train_speed.speed;
            train_state[train] = (train_state[train] & ~TRAIN_SPEED_MASK) | speed;
            marklin_train_ctl(marklin_server, train, train_state[train]);
			break;
		}
        case PARSER_RESULT_REVERSE: {
            uint32_t train = command._data.reverse.train;

            marklin_train_ctl(marklin_server, train, SPEED_STOP);
            Delay(clock_server, 10); // TODO arbritrary delay
            marklin_train_ctl(marklin_server, train, SPEED_REVERSE);
            Delay(clock_server, 10); // TODO arbritrary delay
            marklin_train_ctl(marklin_server, train, train_state[train]);

			break;
		}
        case PARSER_RESULT_SWITCH: {
            u32 switch_id = command._data.switch_control.switch_id;
            SwitchMode switch_mode = command._data.switch_control.switch_mode;
            marklin_switch_ctl(marklin_server, switch_id, switch_mode);
			break;
		}
        case PARSER_RESULT_STOP: {
            marklin_stop(marklin_server);
			break;
		}
        case PARSER_RESULT_GO: {
            marklin_go(marklin_server);
			break;
		}
        case PARSER_RESULT_LIGHTS: {
            uint32_t train = command._data.lights.train;
            if (command._data.lights.state) {
                train_state[train] |= TRAIN_LIGHTS_MASK;
            } else {
                train_state[train] &= ~TRAIN_LIGHTS_MASK;
            }
            marklin_train_ctl(marklin_server, train, train_state[train]);
			break;
		}
        case PARSER_RESULT_QUIT: {
            // TODO exit task and restore console state
            Exit();
			break; // unreachable
		}
        default: {
            ULOG_WARN("Parser result was invalid");
        }
    }
}

// soley responsible for rendering the ui
void
uiTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid render_tid = Create(2, &renderTask, "render task");

    Tid prompt_tid = Create(2, &promptTask, "prompt task");
    Tid switch_state_tid = Create(2, &switchStateTask, "switch state task");

    WaitTid(prompt_tid);

    //  TODO impleement Kill() syscall

    Exit();
}
