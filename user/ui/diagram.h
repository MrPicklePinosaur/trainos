#ifndef __UI_DIAGRAM_H__
#define __UI_DIAGRAM_H__

const char* TRACK_DIAGRAM[] = {
"╭         ╭──A──╮                         ╭─────╮                                         ╭─────╮ ╭─────╮",
"├─────────┤     ├──── ─────── ────────────┤     ├─────────────────────────────────────────┤     ├─┤     ├───╮",
"╰         ╰─────╯    │       │            ╰─────╯                                         ╰─────╯ ╰─────╯   │",
"╭         ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮   │",
"├─────────┤     ├───  16   15  ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├──",
"╰         ╰─────╯    │       │            ╰─────╯         ╰─────╯         ╰─────╯         ╰─────╯ ╰─────╯   │",
"╭         ╭─────╮    │    ╭──┴──╮                 ╭──┴──╮ ╭─────╮         ╭─────╮ ╭──┴──╮                   │",
"├─────────┤     ├────╯    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                   │",
"╰         ╰─────╯         ╰──┬──╯                 ╰─────╯ ╰─────╯    │    ╰─────╯ ╰─────╯                   │",
"╭ ╭─────╮                 ╭──┴──╮                 ╭─────╮ ╭─────╮    │    ╭─────╮ ╭─────╮                   │",
"├─┤     ├────────────╮    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                   │",
"╰ ╰─────╯            │    ╰──┬──╯                 ╰──┬──╯ ╰─────╯         ╰─────╯ ╰──┬──╯                   │",
"╭ ╭─────╮ ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮   │",
"├─┤     ├─┤     ├───           ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├──",
"╰ ╰─────╯ ╰─────╯    │       │            ╰─────╯         ╰─────╯         ╰─────╯         ╰─────╯ ╰─────╯   │",
"╭ ╭─────╮ ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮   │",
"├─┤     ├─┤     ├───         ╰────────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├───╯",
"╰ ╰─────╯ ╰─────╯    │                    ╰─────╯    │    ╰─────╯         ╰─────╯    │    ╰─────╯ ╰─────╯",
"╭ ╭─────╮ ╭─────╮    │                    ╭─────╮    │                               │    ╭─────╮           ╮",
"├─┤     ├─┤     ├──── ────────────────────┤     ├──── ─────────────────────────────── ────┤     ├───────────┤",
"╰ ╰─────╯ ╰─────╯                         ╰─────╯                                         ╰─────╯           ╯",
0
};

const char* TRACK_DIAGRAM_TALL[] = {
"          ╭──A──╮                         ╭─────╮                                         ╭─────╮ ╭─────╮",
"╭         │01/10│                         │     │                                         │     │ │     │",
"├─────────┤     ├──── ─────── ────────────┤     ├─────────────────────────────────────────┤     ├─┤     ├───╮",
"╰         │ 4 7 │                         │     │                                         │     │ │     │   │",
"          ╰─────╯    │       │            ╰─────╯                                         ╰─────╯ ╰─────╯   │",
"          ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮   │",
"╭         │     │    │       │            │     │         │     │         │     │         │     │ │     │   │",
"├─────────┤     ├───  16   15  ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├──",
"╰         │     │    │       │            │     │         │     │         │     │         │     │ │     │   │",
"          ╰─────╯    │       │            ╰─────╯    │    ╰─────╯         ╰─────╯    │    ╰─────╯ ╰─────╯   │",
"          ╭─────╮    │    ╭──┴──╮                 ╭──┴──╮ ╭─────╮         ╭─────╮ ╭──┴──╮                   │",
"╭         │     │    │    │     │                 │     │ │     │         │     │ │     │                   │",
"├─────────┤     ├────╯    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                   │",
"╰         │     │         │     │                 │     │ │     │    │    │     │ │     │                   │",
"          ╰─────╯         ╰──┬──╯                 ╰─────╯ ╰─────╯    │    ╰─────╯ ╰─────╯                   │",
"  ╭─────╮                 ╭──┴──╮                 ╭─────╮ ╭─────╮    │    ╭─────╮ ╭─────╮                   │",
"╭ │     │                 │     │                 │     │ │     │    │    │     │ │     │                   │",
"├─┤     ├────────────╮    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                   │",
"╰ │     │            │    │     │                 │     │ │     │         │     │ │     │                   │",
"  ╰─────╯            │    ╰──┬──╯                 ╰──┬──╯ ╰─────╯         ╰─────╯ ╰──┬──╯                   │",
"  ╭─────╮ ╭─────╮    │       │            ╭─────╮    │    ╭─────╮         ╭─────╮    │    ╭─────╮ ╭─────╮   │",
"╭ │     │ │     │    │       │            │     │         │     │         │     │         │     │ │     │   │",
"├─┤     ├─┤     ├───           ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├──",
"╰ │     │ │     │    │       │            │     │         │     │         │     │         │     │ │     │   │",
"  ╰─────╯ ╰─────╯    │       │            ╰─────╯         ╰─────╯         ╰─────╯         ╰─────╯ ╰─────╯   │",
"  ╭─────╮ ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮   │",
"╭ │     │ │     │    │       │            │     │         │     │         │     │         │     │ │     │   │",
"├─┤     ├─┤     ├───         ╰────────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├───╯",
"╰ │     │ │     │    │                    │     │         │     │         │     │         │     │ │     │",
"  ╰─────╯ ╰─────╯    │                    ╰─────╯    │    ╰─────╯         ╰─────╯    │    ╰─────╯ ╰─────╯",
"  ╭─────╮ ╭─────╮    │                    ╭─────╮    │                               │    ╭─────╮",
"╭ │     │ │     │                         │     │                                         │     │           ╮",
"├─┤     ├─┤     ├──── ────────────────────┤     ├──── ─────────────────────────────── ────┤     ├───────────┤",
"╰ │     │ │     │                         │     │                                         │     │           ╯",
"  ╰─────╯ ╰─────╯                         ╰─────╯                                         ╰─────╯",
0
};



const char* TRACK_DIAGRAM_TALL_SPACE[] = {
"╭         ╭─────╮    12      11           ╭─────╮                                         ╭─────╮ ╭─────╮",
"├─────────┤     ├──── ─────── ────────────┤     ├─────────────────────────────────────────┤     ├─┤     ├────╮",
"╰         ╰─────╯                         ╰─────╯                                         ╰─────╯ ╰─────╯    │",
"                     │       │                                                                               │",
"╭         ╭─────╮    │       │            ╭─────╮    13   ╭─────╮         ╭─────╮    10   ╭─────╮ ╭─────╮    │",
"├─────────┤     ├───  04   14  ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├───  09",
"╰         ╰─────╯    │       │            ╰─────╯         ╰─────╯         ╰─────╯         ╰─────╯ ╰─────╯    │",
"                     │       │                       │                               │                       │",
"╭         ╭─────╮    │    ╭──┴──╮                 ╭──┴──╮ ╭─────╮ 156/155 ╭─────╮ ╭──┴──╮                    │",
"├─────────┤     ├────╯    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                    │",
"╰         ╰─────╯         ╰──┬──╯                 ╰─────╯ ╰─────╯    │    ╰─────╯ ╰─────╯                    │",
"                             │                                       │                                       │",
"╭ ╭─────╮                 ╭──┴──╮                 ╭─────╮ ╭─────╮    │    ╭─────╮ ╭─────╮                    │",
"├─┤     ├────────────╮    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                    │",
"╰ ╰─────╯            │    ╰──┬──╯                 ╰──┬──╯ ╰─────╯ 153/154 ╰─────╯ ╰──┬──╯                    │",
"                     │       │                       │                               │                       │",
"╭ ╭─────╮ ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮    │",
"├─┤     ├─┤     ├───  01   15  ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├───  08",
"╰ ╰─────╯ ╰─────╯    │       │            ╰─────╯    16   ╰─────╯         ╰─────╯    17   ╰─────╯ ╰─────╯    │",
"                     │       │                                                                               │",
"╭ ╭─────╮ ╭─────╮    │       │            ╭─────╮    06   ╭─────╮         ╭─────╮    07   ╭─────╮ ╭─────╮    │",
"├─┤     ├─┤     ├───  02     ╰────────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├────╯",
"╰ ╰─────╯ ╰─────╯    │                    ╰─────╯         ╰─────╯         ╰─────╯         ╰─────╯ ╰─────╯",
"                     │                               │                               │",
"╭ ╭─────╮ ╭─────╮    │                    ╭─────╮                                         ╭─────╮            ╮",
"├─┤     ├─┤     ├──── ────────────────────┤     ├──── ─────────────────────────────── ────┤     ├────────────┤",
"╰ ╰─────╯ ╰─────╯    03                   ╰─────╯    18                              05   ╰─────╯            ╯",
0
};

typedef enum {
    LEFT = 0,
    UP,
    RIGHT,
    DOWN
} Direction;

typedef struct {
    u32 sensor;
    u32 x;
    u32 y;
    Direction direction1;  // Direction of lower sensor number
    Direction direction2;  // Direction of higher sensor number
} DiagramData;

#define DIAGRAM_NODE_COUNT 40
#define DIAGRAM_SWITCH_COUNT 19

static const DiagramData diagram_data[DIAGRAM_NODE_COUNT] = {
    {0, 1, 0, RIGHT, LEFT},  // A1
    {2, 3, 2, UP, DOWN},  // A3
    {4, 1, 6, RIGHT, LEFT},  // A5
    {6, 1, 5, LEFT, RIGHT},  // A7
    {8, 1, 4, LEFT, RIGHT},  // A9
    {10, 0, 3, RIGHT, LEFT},  // A11
    {12, 1, 1, RIGHT, LEFT},  // A13
    {14, 1, 2, LEFT, RIGHT},  // A15
    {16, 7, 4, RIGHT, LEFT},  // B1
    {18, 6, 3, RIGHT, DOWN},  // B3
    {20, 7, 1, RIGHT, LEFT},  // B5
    {22, 0, 4, RIGHT, LEFT},  // B7
    {24, 0, 6, RIGHT, LEFT},  // B9
    {26, 0, 5, RIGHT, LEFT},  // B11
    {28, 9, 3, LEFT, RIGHT},  // B13
    {30, 3, 3, UP, DOWN},  // B15
    {32, 7, 3, LEFT, RIGHT},  // C1
    {34, 11, 6, RIGHT, LEFT},  // C3
    {36, 5, 5, RIGHT, LEFT},  // C5
    {38, 5, 6, RIGHT, LEFT},  // C7
    {40, 5, 4, LEFT, RIGHT},  // C9
    {42, 5, 1, RIGHT, LEFT},  // C11
    {44, 5, 0, RIGHT, LEFT},  // C13
    {46, 7, 5, RIGHT, LEFT},  // C15
    {48, 9, 2, LEFT, RIGHT},  // D1
    {50, 9, 1, RIGHT, LEFT},  // D3
    {52, 12, 1, LEFT, RIGHT},  // D5
    {54, 12, 0, RIGHT, LEFT},  // D7
    {56, 12, 5, LEFT, RIGHT},  // D9
    {58, 9, 5, LEFT, RIGHT},  // D11
    {60, 9, 4, LEFT, RIGHT},  // D13
    {62, 10, 3, LEFT, DOWN},  // D15
    {64, 7, 2, RIGHT, LEFT},  // E1
    {66, 10, 2, LEFT, UP},  // E3
    {68, 11, 1, RIGHT, LEFT},  // E5
    {70, 11, 0, RIGHT, LEFT},  // E7
    {72, 12, 4, RIGHT, LEFT},  // E9
    {74, 11, 5, RIGHT, LEFT},  // E11
    {76, 11, 4, LEFT, RIGHT},  // E13
    {78, 6, 2, UP, RIGHT},  // E15
};

typedef struct {
    u32 switch_id;
    u32 x;
    u32 y;
    Direction major;  // Direction of merge
    Direction minor;  // Direction of branch that is non-parallel to merge
} DiagramSwitchData;

static const DiagramSwitchData diagram_switch_data[DIAGRAM_SWITCH_COUNT] = {
    {},  // Dummy
    {1, 2, 4, DOWN, LEFT},  // 1
    {2, 2, 5, DOWN, LEFT},  // 2
    {3, 2, 6, RIGHT, UP},  // 3
    {4, 2, 1, UP, LEFT},  // 4
    {5, 10, 6, LEFT, UP},  // 5
    {6, 6, 5, LEFT, DOWN},  // 6
    {7, 10, 5, RIGHT, DOWN},  // 7
    {8, 13, 4, UP, LEFT},  // 8
    {9, 13, 1, DOWN, LEFT},  // 9
    {10, 10, 1, RIGHT, DOWN},  // 10
    {11, 3, 0, RIGHT, DOWN},  // 11
    {12, 2, 0, RIGHT, DOWN},  // 12
    {13, 6, 1, LEFT, DOWN},  // 13
    {14, 3, 1, DOWN, RIGHT},  // 14
    {15, 3, 4, UP, RIGHT},  // 15
    {16, 6, 4, LEFT, UP},  // 16
    {17, 10, 4, RIGHT, UP},  // 17
    {18, 6, 6, RIGHT, UP},  // 18
};

#endif // __UI_DIAGRAM_H__
