#include <trainstd.h>
#include <trainsys.h>
#include <trainterm.h>
#include <traintasks.h>
#include <ctype.h>
#include "user/path/path.h"
#include "user/path/train_data.h"
#include "user/path/reserve.h"
#include "user/sensor.h"
#include "user/switch.h"
#include "parser.h"
#include "prompt.h"
#include "render.h"
#include "user/trainstate.h"

#include "kern/perf.h"

void executeCommand(Arena tmp, Tid marklin_server, Tid clock_server, Tid renderer_server, Tid switch_server, Tid trainstate_server, Tid reserve_server, ParserResult command);

// task for getting user input form the console
void
promptTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_CONSOLE);
    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid renderer_server = WhoIs(RENDERER_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid reserve_server = WhoIs(RESERVE_ADDRESS);

    CBuf* line = cbuf_new(32);

    // TODO this is dumb hack to make sure any strings generated by parser are preserved
    Arena parser_arena = arena_new(256);
    // TODO: by using a temp arena, we need to ensure that the console server reads the line we send it and copy the data before we write the next line. Otherwise we will end up with duplicate lines.
    Arena tmp_arena = arena_new(256);

    for (;;) {
        int c = Getc(io_server);

        renderer_prompt(renderer_server, c);

        if (isalnum(c) || isblank(c) || isprint(c)) {
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
            executeCommand(tmp_arena, marklin_server, clock_server, renderer_server, switch_server, trainstate_server, reserve_server, parsed);

        } else if (c == CH_BACKSPACE) {
            cbuf_pop_back(line);
        }

    }
}

void
executeCommand(Arena tmp, Tid marklin_server, Tid clock_server, Tid renderer_server, Tid switch_server, Tid trainstate_server, Tid reserve_server, ParserResult command)
{
    switch (command._type) {
        case PARSER_RESULT_TRAIN_SPEED: {
            uint32_t train = command._data.train_speed.train;
            uint32_t speed = command._data.train_speed.speed;

            char* msg = cstr_format(&tmp, "Setting train %s%d%s to speed %s%d%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, speed, ANSI_RESET);
            renderer_append_console(renderer_server, msg);
            TrainstateSetSpeed(trainstate_server, train, speed);

			break;
		}
        case PARSER_RESULT_REVERSE: {
            uint32_t train = command._data.reverse.train;

            char* msg = cstr_format(&tmp, "Reversing train %s%d%s", ANSI_CYAN, train, ANSI_CYAN);
            renderer_append_console(renderer_server, msg);
            TrainstateReverse(trainstate_server, train);

			break;
		}
        case PARSER_RESULT_SWITCH: {
            u32 switch_id = command._data.switch_control.switch_id;
            SwitchMode switch_mode = command._data.switch_control.switch_mode;

            char* msg = cstr_format(&tmp, "Setting switch %s%x%s to %s%s%s", ANSI_CYAN, switch_id, ANSI_RESET, ANSI_GREEN, (switch_mode == SWITCH_MODE_CURVED) ? "curved" : "straight", ANSI_RESET);
            renderer_append_console(renderer_server, msg);

            SwitchChange(switch_server, switch_id, switch_mode);
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

            TrainstateSetLights(trainstate_server, train, light_state);

			break;
		}
        case PARSER_RESULT_QUIT: {
            // exit task and restore console state
            term_clear();
            Exit();
			break; // unreachable
		}
        case PARSER_RESULT_PATH: {
            u32 train = command._data.path.train;
            char* dest = command._data.path.dest;
            u32 speed = command._data.path.speed;
            i32 offset = command._data.path.offset;

            if (
                speed == TRAIN_SPEED_SNAIL ||
                speed == TRAIN_SPEED_LOW ||
                speed == TRAIN_SPEED_MED ||
                speed == TRAIN_SPEED_HIGH
            ) {
                char* msg = cstr_format(&tmp, "Sending train %s%d%s to %s%s%s at speed %s%d%s with offset %s%d%s",
                    ANSI_CYAN, train, ANSI_RESET,
                    ANSI_GREEN, dest, ANSI_RESET,
                    ANSI_GREEN, speed, ANSI_RESET,
                    ANSI_GREEN, offset, ANSI_RESET
                );
                renderer_append_console(renderer_server, msg);

                PlanPath((Path){train, speed, offset, dest});
            }
            else {
                char* msg = cstr_format(&tmp, "Invalid speed, must be %s%d%s, %s%d%s, %s%d%s, or %s%d%s",
                    ANSI_GREEN, TRAIN_SPEED_SNAIL, ANSI_RESET,
                    ANSI_GREEN, TRAIN_SPEED_LOW, ANSI_RESET,
                    ANSI_GREEN, TRAIN_SPEED_MED, ANSI_RESET,
                    ANSI_GREEN, TRAIN_SPEED_HIGH, ANSI_RESET
                );
                renderer_append_console(renderer_server, msg);
            }
			break;
        } 
        case PARSER_RESULT_TEST: {
            switch (command._data.test.num) {
                case 1: {
                    renderer_append_console(renderer_server, "Running benchmark 1");

                    Track* track = get_track();
                    TrackNode* node = 0;
                    usize SPEED = 5;
                    
                    // Train 2 A5/6 -> E7/8
                    node = track_node_by_name(track, "A5");
                    TrainstateSetPos(trainstate_server, reserve_server, 2, node);
                    Path train1_paths[] = {(Path){2, SPEED, 0, "E8"}, (Path){2, SPEED, 0, "A5"}};
                    Tid train1_pather = PlanPathSeq(train1_paths, 2);

                    // Train 47 C3/4 -> A3/4
                    node = track_node_by_name(track, "C4");
                    TrainstateSetPos(trainstate_server, reserve_server, 47, node);
                    Path train2_paths[] = {(Path){47, SPEED, 0, "A3"}, (Path){47, SPEED, 0, "C4"}};
                    Tid train2_pather = PlanPathSeq(train2_paths, 2);

                    WaitTid(train1_pather);
                    WaitTid(train2_pather);

                    break;
                }
                default:
                    renderer_append_console(renderer_server, "Invalid test");
            }
            break;
        }
        case PARSER_RESULT_POS: {
            // TODO currently can only set position to a sensor

            usize train = command._data.pos.train;
            Track* track = get_track();
            TrackNode* node = track_node_by_name(track, command._data.pos.pos);

            char* msg = cstr_format(&tmp, "Sending train %s%d%s position to %s%s%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, node->name, ANSI_RESET);
            renderer_append_console(renderer_server, msg);
            TrainstateSetPos(trainstate_server, reserve_server, command._data.pos.train, node);

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
