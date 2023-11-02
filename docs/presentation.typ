
#import "@preview/polylux:0.3.1": *

#let trainos-theme(aspect-ratio: "16-9", body) = {
  let background-color = rgb("#350035")

  set page(
    paper: "presentation-" + aspect-ratio,
    fill: background-color,
    margin: (top: 4%, right: 4%, bottom: 4%, left: 4%)
  )
  set text(fill: white.darken(10%), size: 40pt, font: "Fira Sans")

  body
}
#show: trainos-theme.with()

#set raw(theme: "trainos.tmTheme")
#show raw: it => block(
  text(fill: rgb("#ffffff"), it)
)

#set page(paper: "presentation-16-9")
#set text(size: 22pt)

#polylux-slide[
  #align(horizon + center)[
    #image("logo-colored.png", width: 40%)

    Daniel Liu and Joey Zhou
  ]
]


#polylux-slide[
    = Directory Structure

    ```
    .
    ├── include/
    │   └── ...
    ├── kern/
    │   └── ...
    ├── lib/
    │   ├── trainstd/
    │   ├── trainsys/
    │   └── trainterm/
    └── user/
        └── tests/
    ```
]

#polylux-slide[
    = Memory Allocation

    Free-list based malloc for arbritrary lifetimes
    ```c
    SchedulerNode* node = kalloc(sizeof(SchedulerNode));
    kfree(node);
    ```
]

#polylux-slide[
    = Memory Allocation

    Arena allocator for grouped lifetimes
    #set text(20pt)
    ```c
    void do_stuff(Arena* arena, Arena tmp) {
      cstr_format(&tmp, "Reversing train %d", train);
      u32* ret = arena_alloc(arena, u32);
      *ret = 69;
    }

    Arena arena = arena_new(256);
    Arena tmp = arena_new(256);

    do_stuff(&arena, tmp);
    // after return: arena preserved, tmp not preserved

    arena_release(&arena);
    arena_release(&tmp);
    ```
]

#polylux-slide[
    = trainstd.h: the standard library

    - General use functions
    - Data structures
      - Linked list
      - Hash map
      - Circular buffer
]

#polylux-slide[
    = trainterm.h: ncurses inspired ui library

    Primitive terminal drawing library with ncurses inspired API. Currently no cursor movement optimizations.
    ```c
    Window win = win_init(2, 2, 20, 10);
    win_draw(&win);

    w_attr(ATTR_RED);
    w_puts_mv("my window", 2, 2);
    w_attr_reset();
    ```
]

#polylux-slide[
    = Tasks & Task Table

    #side-by-side[
      #set text(18pt)
      ```c
      typedef struct {
          SwitchFrame* sf;
          Tid tid;
          Tid parent_tid;
          const char* name;

          TaskState state;
          u32 priority;
          Addrspace addrspace;

          CBuf* receive_queue;
          SendBuf* send_buf;
          ReceiveBuf* receive_buf;

          EventId blocking_event;
      } Task;
      ```
    ][
      - Heap allocated
      - Switch frame struct at top, also heap allocated
      - State and priority for scheduler
      - Fields for send and receive
      - Accessed through task table, a hash map
    ]
]

#polylux-slide[
    = Scheduler

    - Hash map of tasks
    - Hash function is priority modulo 16
    - Find next task by searching through all tasks, skipping over blocked tasks
]

#polylux-slide[
    = Address space

    - Each task gets a 1mb page to use as a stack
    - Page addresses stored in array, max 256 pages at once

    ```c
    typedef struct {
        Address base;
        Address stackbase;
    } Addrspace;


    static unsigned char* const USER_BASE = (unsigned char*)0x00220000;
    static const unsigned int USER_ADDRSPACE_SIZE = 0x00010000; // 1mb

    Address base = USER_BASE + USER_ADDRSPACE_SIZE * i;
    ```
]

#polylux-slide[
    = Context Switch: Entering kernel

    #side-by-side[
      #set text(18pt)
      ```asm
      # save all registers to switchframe struct
      str x1, [x0, #8]
      stp x2, x3, [x0, #16]
      ... 
      stp x28, x30, [x0, #224]

      # save stack ptr
      mrs x1, SP_EL0
      str x1, [x0, #240]

      # load return value
      mrs x1, ELR_EL1
      str x1, [x0, #248]

      # load state reg
      mrs x1, SPSR_EL1
      str x1, [x0, #256]

      b handle_svc
      ```
    ][
      - Don't need to save kernel state
    ]

]

#polylux-slide[
    = Context Switch: Exiting kernel

    #side-by-side[
      #set text(16pt)
      ```asm
      ldp x2, x3, [x0, #16]
      ...
      ldp x28, x30, [x0, #224]

      # load stack ptr
      ldr x1, [x0, #240]
      msr SP_EL0, x1

      # load return value
      ldr x1, [x0, #248]
      msr ELR_EL1, x1

      # load state reg
      ldr x1, [x0, #256]
      msr SPSR_EL1, x1

      # reset kernel stack ptr
      mov sp, #0x00200000

      ldp x0, x1, [x0, #0]

      eret
      ```
    ][
      - Notice that we need to reset kernel stack pointer (more on this later)
    ]

]
#polylux-slide[
    = Kernel Events

    ```c
    typedef enum {
        EVENT_NONE = 0,
        EVENT_CLOCK_TICK,
        EVENT_MARKLIN_RX,
        EVENT_MARKLIN_CTS,
        EVENT_CONSOLE_RX,
        EVENT_TASK_EXIT,
    } EventId;
    ```
]

#polylux-slide[
    = Message Passing

    - Messages are structs
    - Tasks are blocked while waiting for a message
    - If waiting for a Receive(), Send() messages are stored in task table entry
    - Each task has a linked list of other tasks waiting for it to call Receive()
]

#polylux-slide[
    = Essential Tasks

    - Init
    - Nameserver
    - Clock  
    - IO
    - Idle
]

#polylux-slide[
    = Essential Tasks: Init

    - Spawns other essential servers
    - Prompts user with menu of tasks to run

    ```
    ================= SELECT TASK TO RUN =================
    [0] K1
    [1] K2
    [2] K2Perf
    [3] K3
    [4] K4
    [5] sendReceiveReplyTest
    [6] graphics
    [7] test
    ======================================================
    ```
]

#polylux-slide[
    = Essential Tasks: Nameserver

    - A linked list of all named tasks, linear search to retrieve them
    - WhoIs call will retry a couple times
]

#polylux-slide[
    = Essential Tasks: Clock

    - Notifier pattern
    - The task id as well as the target time is pushed into a list
    - Each tick, any delay requests that have been reached are replied to

]

#polylux-slide[
    = Essential Tasks: IO

    - One server for Marklin, one server for console
    - Marklin RX, CTS
    - Console RX

    - One notifier task per interrupt type
]

#polylux-slide[
    = Essential Tasks: Idle

    - Loops on WFI instruction
    - Kernel tracks percentage of runtime spent idling, perf task will fetch this data periodically
]

// #polylux-slide[
//     = MarklinCTL: train control TUI


// ]

#polylux-slide[
    = Testing

    Test suite for testing code
    #set text(18pt)
    ```c
    void testCbuf() {
      CBuf* out_stream = cbuf_new(10);
      TEST(cbuf_len(out_stream) == 0);

      cbuf_push_front(out_stream, 0x1);
      TEST(cbuf_len(out_stream) == 1);

      cbuf_push_front(out_stream, 0x2);
      cbuf_push_front(out_stream, 0x3);
      TEST(cbuf_len(out_stream) == 3);

      u8 val = cbuf_pop_front(out_stream);
      TEST(cbuf_len(out_stream) == 2);
      TEST(val == 0x3);
    }
    ```
]

#polylux-slide[
    = Logging

    Multiple log levels and log masks to filter output
    ```c
    set_log_level(LOG_LEVEL_INFO);
    set_log_mask(LOG_MASK_KERN|LOG_MASK_USER|LOG_MASK_IO);

    ULOG_INFO_M(LOG_MASK_IO, "this log message is printed");
    ULOG_INFO_M(LOG_MASK_PARSER, "this log message is not printed");
    ```
]

#polylux-slide[
    = QEMU

    - Only supports up to RPi 3B.
    - Use some conditional compilation to work with RPi 3B memory layout.
    - Interrupts don't work D:
]

#polylux-slide[
    = Gacha

    #image("gacha.jpg", width: 60%)
]
