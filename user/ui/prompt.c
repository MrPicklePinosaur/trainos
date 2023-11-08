#include <trainstd.h>
#include <trainsys.h>
#include <trainterm.h>
#include <ctype.h>
#include "user/clock.h"
#include "user/io.h"
#include "user/path/path.h"
#include "user/nameserver.h"
#include "user/sensor.h"
#include "parser.h"
#include "prompt.h"
#include "render.h"

#include "kern/perf.h"

#define NUMBER_OF_TRAINS 80
#define TRAIN_SPEED_MASK     0b01111
#define TRAIN_LIGHTS_MASK    0b10000

void executeCommand(Arena tmp, Tid marklin_server, Tid clock_server, Tid renderer_server, Tid path_server, TrainState* train_state, ParserResult command);

// task for getting user input form the console
void
promptTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_CONSOLE);
    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid renderer_server = WhoIs(RENDERER_ADDRESS);
    Tid path_server = WhoIs(PATH_ADDRESS);

    TrainState train_state[NUMBER_OF_TRAINS] = {0};

    CBuf* line = cbuf_new(32);

    // TODO this is dumb hack to make sure any strings generated by parser are preserved
    Arena parser_arena = arena_new(256);
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
            ParserResult parsed = parse_command(parser_arena, str8(completed_line));
            executeCommand(tmp_arena, marklin_server, clock_server, renderer_server, path_server, train_state, parsed);

        } else if (c == CH_BACKSPACE) {
            cbuf_pop_back(line);
        }

    }
}

void
executeCommand(Arena tmp, Tid marklin_server, Tid clock_server, Tid renderer_server, Tid path_server, TrainState* train_state, ParserResult command)
{
    switch (command._type) {
        case PARSER_RESULT_TRAIN_SPEED: {
            uint32_t train = command._data.train_speed.train;
            uint32_t speed = command._data.train_speed.speed;
            train_state[train] = (train_state[train] & ~TRAIN_SPEED_MASK) | speed;

            char* msg = cstr_format(&tmp, "Setting train %s%d%s to speed %s%d%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, speed, ANSI_RESET);
            renderer_append_console(renderer_server, msg);
            marklin_train_ctl(marklin_server, train, train_state[train]);

			break;
		}
        case PARSER_RESULT_REVERSE: {
            uint32_t train = command._data.reverse.train;

            char* msg = cstr_format(&tmp, "Reversing train %s%d%s", ANSI_CYAN, train, ANSI_CYAN);
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

            char* msg = cstr_format(&tmp, "Setting switch %s%x%s to %s%s%s", ANSI_CYAN, switch_id, ANSI_RESET, ANSI_GREEN, (switch_mode == SWITCH_MODE_CURVED) ? "curved" : "straight", ANSI_RESET);
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

            char* msg = cstr_format(&tmp, "Turned lights on train %s%d%s %s%s%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, (light_state) ? "on" : "off", ANSI_RESET);
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
        case PARSER_RESULT_PATH: {
            uint32_t train = command._data.path.train;
            char* dest = command._data.path.dest;

            char* msg = cstr_format(&tmp, "Sending train %s%d%s to %s%s%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, dest, ANSI_RESET);
            renderer_append_console(renderer_server, msg);

            PlanPath(path_server, train, dest);

			break;
        } 
        default: {
            renderer_append_console(renderer_server, "Invalid command");
        }
    }
}

void
trackDiagram()
{
#if 0

   ▕ ▕ ▕ ▕       ▕ ▕ ▕ 
   ▕ ▕ ▕ ╱        ╲▕ ▕   
   ▕ ▕ ▕╱          ╲ ▕  
   ▕ ▕ ╱            ╲▕ 
   ▕ ▕╱ ╱▔╱▔▔▔▔▔▔╲▔╲ ╲
   ▕ ╱ ╱ ╱        ╲ ╲▕ 
   ▕╱ ╱ ╱          ╲ ╲  
   ▕ ▕ ▕           ▕ ▕       
   ▕ ╱  ╲          ╱ ▕ 
    ╱▕ ▕ ╲        ╱▕ ▕ 
   ▕ ▕ ▕  ╲▁▁▁▁▁▁╱ ▕ ▕ 
   ▕ ▕ ▕  ╱      ╲ ▕ ▕ 
    ╲▕ ▕ ╱        ╲▕ ▕ 
   ▕ ╲  ╱          ╲ ▕ 
   ▕ ▕ ▕           ▕ ▕             
   ▕  ╲ ╲          ╱ ╱
   ▕   ╲ ╲        ╱ ╱
        ╲▁╲▁▁▁▁▁▁╱▁╱


  ---------------------     ------------------------------------------------###
  --------------------------------------------------------------------------#  ##      
  ----------------        #####             ###             ####             #####     
                         ###                   ###       ###                   #####   
                       ###                        ## | ###                       ####  
                      ##                           ##|##                          ###  
                      ##                            #|#                            ##  
                      ##                             |                             ##  
                      ##                             |                             ##  
                      ##                             |                             ##  
                      ##                            #|#                            ##  
                      ##                            #|##                           ##  
                       ##                         ###| ##                         ###  
                        ###                     ###  |  ###                     ####   
   --------------         ####               ###           # #               ######    
   -----------------        ##----------------------------------------------# ###      
   -------------------------------    -----------------------------------------             
   --------------------------------------------------------------------------------         

  ═══════════╦═════╦═══════════════════════════════════╗
  ══════╭════╝     ╠════════╮══════════════════════════╣
  ══════╯          ║        ║ 
                   ║        ║
                   ║        ║
                   ║        ║
  ══════╗          ║
  ══════╩════╗     ╚═══      
  ═══════════╩════╗      ╚════╦══════════╦═════════════════════════
  ════════════════╩═══════════╩══════════╩══════════════════════════════
#endif

}
