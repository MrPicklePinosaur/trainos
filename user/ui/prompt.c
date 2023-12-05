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

void executeCommand(Arena* tmp, Tid marklin_server, Tid clock_server, Tid renderer_server, Tid switch_server, Tid trainstate_server, Tid reserve_server, ParserResult command);

// task for getting user input form the console
void
promptTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_CONSOLE);
    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid prompt_renderer_server = WhoIs(PROMPT_ADDRESS);
    Tid console_renderer_server = WhoIs(CONSOLE_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid reserve_server = WhoIs(RESERVE_ADDRESS);

    CBuf* line = cbuf_new(128);

    // TODO this is dumb hack to make sure any strings generated by parser are preserved
    Arena parser_arena = arena_new(256);
    // TODO: by using a temp arena, we need to ensure that the console server reads the line we send it and copy the data before we write the next line. Otherwise we will end up with duplicate lines.
    Arena tmp_base = arena_new(256);

    for (;;) {
        Arena tmp = tmp_base;

        int c = Getc(io_server);

        renderer_prompt(prompt_renderer_server, c);

        if ((isalnum(c) || isblank(c) || isprint(c)) && cbuf_len(line) < PROMPT_MAX_LEN) {
            cbuf_push_back(line, (void*)c);
        } else if (c == CH_ENTER) {
            // drain the buffer
            usize input_len = cbuf_len(line);

            if (input_len == 0) continue;

            char completed_line[input_len+1];
            for (u8 i = 0; i < input_len; ++i)
                completed_line[i] = (char)cbuf_get(line, i);

            completed_line[input_len] = 0;
            cbuf_clear(line);

            // it is okay to parse and execute commands synchronously here, since we don't want to print the next prompt line until the command finishes
            // TODO since we are using a tmp arena, we can technically 
            ParserResult parsed = parse_command(parser_arena, str8(completed_line));
            executeCommand(&tmp, marklin_server, clock_server, console_renderer_server, switch_server, trainstate_server, reserve_server, parsed);

        } else if (c == CH_BACKSPACE) {
            cbuf_pop_back(line);
        }

    }
}

void
executeCommand(Arena* tmp, Tid marklin_server, Tid clock_server, Tid console_renderer_server, Tid switch_server, Tid trainstate_server, Tid reserve_server, ParserResult command)
{
    Track* track = get_track();
    switch (command._type) {
        case PARSER_RESULT_TRAIN_SPEED: {
            uint32_t train = command._data.train_speed.train;
            uint32_t speed = command._data.train_speed.speed;

            char* msg = cstr_format(tmp, "Setting train %s%d%s to speed %s%d%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, speed, ANSI_RESET);
            renderer_append_console(console_renderer_server, msg);
            TrainstateSetSpeed(trainstate_server, train, speed);

			break;
		}
        case PARSER_RESULT_REVERSE: {
            uint32_t train = command._data.reverse.train;

            char* msg = cstr_format(tmp, "Reversing train %s%d%s", ANSI_CYAN, train, ANSI_CYAN);
            renderer_append_console(console_renderer_server, msg);
            TrainState state = TrainstateGet(trainstate_server, train);
            TrainstateReverse(trainstate_server, train);
            TrainstateSetSpeed(trainstate_server, train, state.speed);

			break;
		}
        case PARSER_RESULT_SWITCH: {
            u32 switch_id = command._data.switch_control.switch_id;
            SwitchMode switch_mode = command._data.switch_control.switch_mode;

            char* msg = cstr_format(tmp, "Setting switch %s%x%s to %s%s%s", ANSI_CYAN, switch_id, ANSI_RESET, ANSI_GREEN, (switch_mode == SWITCH_MODE_CURVED) ? "curved" : "straight", ANSI_RESET);
            renderer_append_console(console_renderer_server, msg);

            SwitchChange(switch_server, switch_id, switch_mode);
			break;
		}
        case PARSER_RESULT_STOP: {
            renderer_append_console(console_renderer_server, "Powering off marklin");
            marklin_stop(marklin_server);
			break;
		}
        case PARSER_RESULT_GO: {
            renderer_append_console(console_renderer_server, "Powering on marklin");
            marklin_go(marklin_server);
			break;
		}
        case PARSER_RESULT_LIGHTS: {
            uint32_t train = command._data.lights.train;
            bool light_state = command._data.lights.state;

            char* msg = cstr_format(tmp, "Turned lights on train %s%d%s %s%s%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, (light_state) ? "on" : "off", ANSI_RESET);
            renderer_append_console(console_renderer_server, msg);

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
                char* msg = cstr_format(tmp, "Sending train %s%d%s to %s%s%s at speed %s%d%s with offset %s%d%s",
                    ANSI_CYAN, train, ANSI_RESET,
                    ANSI_GREEN, dest, ANSI_RESET,
                    ANSI_GREEN, speed, ANSI_RESET,
                    ANSI_GREEN, offset, ANSI_RESET
                );
                renderer_append_console(console_renderer_server, msg);

                PlanPath((Path){train, speed, offset, dest, false});
            }
            else {
                char* msg = cstr_format(tmp, "Invalid speed, must be %s%d%s, %s%d%s, %s%d%s, or %s%d%s",
                    ANSI_GREEN, TRAIN_SPEED_SNAIL, ANSI_RESET,
                    ANSI_GREEN, TRAIN_SPEED_LOW, ANSI_RESET,
                    ANSI_GREEN, TRAIN_SPEED_MED, ANSI_RESET,
                    ANSI_GREEN, TRAIN_SPEED_HIGH, ANSI_RESET
                );
                renderer_append_console(console_renderer_server, msg);
            }
			break;
        } 
        case PARSER_RESULT_POS: {
            // TODO currently can only set position to a sensor

            usize train = command._data.pos.train;
            Track* track = get_track();
            TrackNode* node = track_node_by_name(track, command._data.pos.pos);

            char* msg = cstr_format(tmp, "Setting train %s%d%s position to %s%s%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, node->name, ANSI_RESET);
            renderer_append_console(console_renderer_server, msg);
            TrainstateSetPos(trainstate_server, reserve_server, command._data.pos.train, node);

            break;
        }
        case PARSER_RESULT_CO: {

            u32 train = command._data.co.train;
            u32 cohort = command._data.co.cohort;

            char* msg = cstr_format(tmp, "Assigning train %s%d%s to cohort %s%d%s", ANSI_CYAN, train, ANSI_RESET, ANSI_GREEN, cohort, ANSI_RESET);
            renderer_append_console(console_renderer_server, msg);

            TrainstateSetCohort(trainstate_server, train, cohort);
            
            break;
        }
        case PARSER_RESULT_TEST: {
            switch (command._data.test.num) {
                case 1: {
                    renderer_append_console(console_renderer_server, "Running benchmark 1: speed test with two trains");

                    TrackNode* node = 0;
                    usize SPEED = 14;
                    
                    usize start_time = Time(clock_server);

                    // Train 2 A5/6 -> E7/8
                    node = track_node_by_name(track, "A5");
                    TrainstateSetPos(trainstate_server, reserve_server, 2, node);
                    Path train1_paths[] = {(Path){2, SPEED, 0, "E8", true}, (Path){2, SPEED, 0, "A5", true}};
                    Tid train1_pather = PlanPathSeq(train1_paths, 2);

                    // Train 47 C3/4 -> A3/4
                    node = track_node_by_name(track, "C4");
                    TrainstateSetPos(trainstate_server, reserve_server, 47, node);
                    Path train2_paths[] = {(Path){47, SPEED, 0, "A3", true}, (Path){47, SPEED, 0, "C4", true}};
                    Tid train2_pather = PlanPathSeq(train2_paths, 2);

                    WaitTid(train1_pather);
                    WaitTid(train2_pather);

                    usize end_time = Time(clock_server);

                    // reverse the two trains so we can run this test again

                    TrainstateReverseStatic(trainstate_server, 2);
                    TrainstateReverseStatic(trainstate_server, 47);

                    char* msg = cstr_format(tmp, "benchmark took %d seconds", (end_time-start_time)/100);
                    renderer_append_console(console_renderer_server, msg);

                    break;
                }
                case 2: {
                    renderer_append_console(console_renderer_server, "Running benchmark 2: reversals");

                    TrackNode* node = 0;
                    usize SPEED = 5;
                    
                    usize start_time = Time(clock_server);

                    // Train 2 C12 -> A5
                    node = track_node_by_name(track, "B15");
                    TrainstateSetPos(trainstate_server, reserve_server, 2, node);
                    Path train1_paths[] = {(Path){2, SPEED, 0, "A5", true}, (Path){2, SPEED, 0, "B15", true}};
                    Tid train1_pather = PlanPathSeq(train1_paths, 2);

                    /* // Train 47 A8 -> C13 */
                    /* node = track_node_by_name(track, "A8"); */
                    /* TrainstateSetPos(trainstate_server, reserve_server, 47, node); */
                    /* Path train2_paths[] = {(Path){47, SPEED, 0, "A4", true}, (Path){47, SPEED, 0, "A8", true}}; */
                    /* Tid train2_pather = PlanPathSeq(train2_paths, 2); */

                    WaitTid(train1_pather);
                    /* WaitTid(train2_pather); */

                    usize end_time = Time(clock_server);

                    char* msg = cstr_format(tmp, "benchmark took %d seconds", (end_time-start_time)/100);
                    renderer_append_console(console_renderer_server, msg);

                    break;
                }
                case 3: {
                    renderer_append_console(console_renderer_server, "Running benchmark 3: four trains");

                    TrackNode* node = 0;
                    usize SPEED = 14;
                    usize TRAIN = 0;
                    
                    usize start_time = Time(clock_server);

                    // Train 2 A5/6 -> E7/8
                    TRAIN = 2;
                    node = track_node_by_name(track, "A5");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train1_paths[] = {(Path){TRAIN, SPEED, 0, "E8", true}, (Path){TRAIN, SPEED, 0, "A5", true}};
                    Tid train1_pather = PlanPathSeq(train1_paths, 2);

                    // Train 47 C3/4 -> A3/4
                    TRAIN = 47;
                    node = track_node_by_name(track, "C4");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train2_paths[] = {(Path){TRAIN, SPEED, 0, "A3", true}, (Path){TRAIN, SPEED, 0, "C4", true}};
                    Tid train2_pather = PlanPathSeq(train2_paths, 2);

                    // Train 58 C11 -> E14
                    TRAIN = 58;
                    node = track_node_by_name(track, "C11");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train3_paths[] = {(Path){TRAIN, SPEED, 0, "E14", true}, (Path){TRAIN, SPEED, 0, "C11", true}};
                    Tid train3_pather = PlanPathSeq(train3_paths, 2);

                    // Train 77 E6 -> B4
                    TRAIN = 77;
                    node = track_node_by_name(track, "E6");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train4_paths[] = {(Path){TRAIN, SPEED, 0, "B4", true}, (Path){TRAIN, SPEED, 0, "E6", true}};
                    Tid train4_pather = PlanPathSeq(train4_paths, 2);

                    WaitTid(train1_pather);
                    WaitTid(train2_pather);
                    WaitTid(train3_pather);
                    WaitTid(train4_pather);

                    usize end_time = Time(clock_server);

                    // reverse the two trains so we can run this test again
                    char* msg = cstr_format(tmp, "benchmark took %d seconds", (end_time-start_time)/100);
                    renderer_append_console(console_renderer_server, msg);

                    TrainstateReverseStatic(trainstate_server, 2);
                    TrainstateReverseStatic(trainstate_server, 47);
                    TrainstateReverseStatic(trainstate_server, 58);
                    TrainstateReverseStatic(trainstate_server, 77);


                    break;
                }
                case 4: {
                    renderer_append_console(console_renderer_server, "Running benchmark 4: random train destinations");

                    createPathRandomizer(2, 5, "A5");
                    createPathRandomizer(47, 5, "C4");

                    break;
                }
                case 5: {
                    renderer_append_console(console_renderer_server, "Running benchmark 5: single train reverse on switch");

                    usize SPEED = 5;

                    TrackNode* node = track_node_by_name(track, "A5");
                    TrainstateSetPos(trainstate_server, reserve_server, 2, node);
                    Tid pather_task = PlanPath((Path){2, SPEED, 0, "A3", true});

                    WaitTid(pather_task);

                    break;
                }
                case 6: {
                    renderer_append_console(console_renderer_server, "Running benchmark 6: move to exit and back out");

                    usize SPEED = 5;
                    usize TRAIN = 2;

                    TrackNode* node = track_node_by_name(track, "E8");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train1_paths[] = {(Path){TRAIN, SPEED, 0, "A14"}, (Path){TRAIN, SPEED, 0, "E8", true}};
                    Tid train1_pather = PlanPathSeq(train1_paths, 2);

                    break;
                }
                case 7: {
                    renderer_append_console(console_renderer_server, "Running benchmark 7: double circles");

                    TrackNode* node = 0;
                    usize SPEED = 5;
                    usize TRAIN = 0;

                    TRAIN = 2;
                    node = track_node_by_name(track, "C9");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train1_paths[] = {
                        (Path){TRAIN, SPEED, 0, "B15", false},
                        (Path){TRAIN, SPEED, 0, "C9", false},
                        (Path){TRAIN, SPEED, 0, "B15", false},
                        (Path){TRAIN, SPEED, 0, "C9", false},
                        (Path){TRAIN, SPEED, 0, "B15", false},
                        (Path){TRAIN, SPEED, 0, "C9", false}
                    };
                    Tid train1_pather = PlanPathSeq(train1_paths, 6);

                    TRAIN = 47;
                    SPEED = 5;
                    node = track_node_by_name(track, "E14");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train2_paths[] = {
                        (Path){TRAIN, SPEED, 0, "E9", false},
                        (Path){TRAIN, SPEED, 0, "E14", false},
                        (Path){TRAIN, SPEED, 0, "E9", false},
                        (Path){TRAIN, SPEED, 0, "E14", false},
                        (Path){TRAIN, SPEED, 0, "E9", false},
                        (Path){TRAIN, SPEED, 0, "E14", false}
                    };
                    Tid train2_pather = PlanPathSeq(train2_paths, 6);

                    break; 
                }
                case 8: {
                    renderer_append_console(console_renderer_server, "Running benchmark 8: three trains cycle");

                    TrackNode* node = 0;
                    usize SPEED = 5;
                    usize TRAIN = 0;

                    // A4 -> D14 -> E6 -> A4
                    TRAIN = 2;
                    node = track_node_by_name(track, "A4");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train1_paths[] = {(Path){TRAIN, SPEED, 0, "D14", false}, (Path){TRAIN, SPEED, 0, "E6", false}, (Path){TRAIN, SPEED, 0, "A4", false}};
                    Tid train1_pather = PlanPathSeq(train1_paths, 3);

                    // D14 -> E6 -> A4 -> D14
                    TRAIN = 47;
                    node = track_node_by_name(track, "D14");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train2_paths[] = {(Path){TRAIN, SPEED, 0, "E6", false}, (Path){TRAIN, SPEED, 0, "A4", false}, (Path){TRAIN, SPEED, 0, "D14", false}};
                    Tid train2_pather = PlanPathSeq(train2_paths, 3);

                    // E6 -> A4 -> D14 -> E6
                    TRAIN = 58;
                    node = track_node_by_name(track, "E6");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train3_paths[] = {(Path){TRAIN, SPEED, 0, "A4", false}, (Path){TRAIN, SPEED, 0, "D14", false}, (Path){TRAIN, SPEED, 0, "E6", false}};
                    Tid train3_pather = PlanPathSeq(train3_paths, 3);
                    
                    WaitTid(train1_pather);
                    WaitTid(train2_pather);
                    WaitTid(train3_pather);

                    break;
                }
                case 9: {
                    renderer_append_console(console_renderer_server, "Running benchmark 9: four trains 2");

                    TrackNode* node = 0;
                    usize SPEED = 14;
                    usize TRAIN = 0;
                    
                    usize start_time = Time(clock_server);

                    // Train 2 A5/6 -> E7/8
                    TRAIN = 2;
                    node = track_node_by_name(track, "A5");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train1_paths[] = {(Path){TRAIN, SPEED, 0, "E8", true}, (Path){TRAIN, SPEED, 0, "A5", true}};
                    Tid train1_pather = PlanPathSeq(train1_paths, 2);

                    // Train 47 C3/4 -> A3/4
                    TRAIN = 47;
                    node = track_node_by_name(track, "C4");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train2_paths[] = {(Path){TRAIN, SPEED, 0, "A3", true}, (Path){TRAIN, SPEED, 0, "C4", true}};
                    Tid train2_pather = PlanPathSeq(train2_paths, 2);

                    // Train 58 C11 -> E14
                    TRAIN = 58;
                    node = track_node_by_name(track, "C11");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train3_paths[] = {(Path){TRAIN, SPEED, 0, "E14", true}, (Path){TRAIN, SPEED, 0, "C11", true}};
                    Tid train3_pather = PlanPathSeq(train3_paths, 2);

                    // Train 77 E5 -> B2
                    TRAIN = 77;
                    node = track_node_by_name(track, "E5");
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN, node);
                    Path train4_paths[] = {(Path){TRAIN, SPEED, 0, "B2", true}, (Path){TRAIN, SPEED, 0, "E5", true}};
                    Tid train4_pather = PlanPathSeq(train4_paths, 2);

                    WaitTid(train1_pather);
                    WaitTid(train2_pather);
                    WaitTid(train3_pather);
                    WaitTid(train4_pather);

                    usize end_time = Time(clock_server);

                    // reverse the two trains so we can run this test again
                    char* msg = cstr_format(tmp, "benchmark took %d seconds", (end_time-start_time)/100);
                    renderer_append_console(console_renderer_server, msg);

                    TrainstateReverseStatic(trainstate_server, 2);
                    TrainstateReverseStatic(trainstate_server, 47);
                    TrainstateReverseStatic(trainstate_server, 58);
                    TrainstateReverseStatic(trainstate_server, 77);


                    break;
                }
                case 0: {

                    // TODO due to the set pos appending to zone_fifo, this test is not re-runnable

                    const usize SPEED = 8;

                    const usize TRAIN1 = 2;
                    const usize TRAIN2 = 47;

                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN1, track_node_by_name(track, "C7"));
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN2, track_node_by_name(track, "A5"));
                    TrainstateSetCohort(trainstate_server, TRAIN2, TRAIN1);

                    // need to offset since we switched the cohort leader on reverse
                    Path cohort1_paths[] = {(Path){TRAIN1, SPEED, 0, "D4", true}, (Path){TRAIN2, SPEED, TRAIN_LENGTH, "A5", true}};
                    Tid cohort1_pather = PlanPathSeq(cohort1_paths, 2);

                    const usize TRAIN3 = 54;
                    const usize TRAIN4 = 58;

                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN3, track_node_by_name(track, "C6"));
                    TrainstateSetPos(trainstate_server, reserve_server, TRAIN4, track_node_by_name(track, "C4"));
                    TrainstateSetCohort(trainstate_server, TRAIN4, TRAIN3);

                    Path cohort2_paths[] = {(Path){TRAIN3, SPEED, 0, "C11", true}, (Path){TRAIN4, SPEED, TRAIN_LENGTH*2, "C4", true}};
                    Tid cohort2_pather = PlanPathSeq(cohort2_paths, 2);

                    WaitTid(cohort1_pather);
                    WaitTid(cohort2_pather);
                    TrainstateReverse(trainstate_server, TRAIN1);
                    TrainstateReverse(trainstate_server, TRAIN3);


                    break;
                }
                default:
                    renderer_append_console(console_renderer_server, "Invalid test");
            }

            break;
        }
        case PARSER_RESULT_HELP: {
            renderer_append_console(console_renderer_server, (char*)"MarklinCTL help =======");
            renderer_append_console(console_renderer_server, (char*)"tr <train> <speed>           set speed of train");
            renderer_append_console(console_renderer_server, (char*)"rv <train>                   reverse a train's direction");
            renderer_append_console(console_renderer_server, (char*)"sw <switch> [S|C]            set the state of a switch");
            renderer_append_console(console_renderer_server, (char*)"stop                         turn off the marklin");
            renderer_append_console(console_renderer_server, (char*)"go                           turn on the marklin");
            renderer_append_console(console_renderer_server, (char*)"lights <train> [on|off]      control lights on train");
            renderer_append_console(console_renderer_server, (char*)"quit                         exit MarklinCTL");
            renderer_append_console(console_renderer_server, (char*)"path <train> <node> <speed>  pathfind to destination");
            renderer_append_console(console_renderer_server, (char*)"test <testnum>               run a predefined test");
            renderer_append_console(console_renderer_server, (char*)"pos <train> <node>           set the position of train");
            renderer_append_console(console_renderer_server, (char*)"co <train> <cohort-id>       assign a train to a cohort");
            renderer_append_console(console_renderer_server, (char*)"help                         print this message");

            break;
        }
        default: {
            renderer_append_console(console_renderer_server, "Invalid command");
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
