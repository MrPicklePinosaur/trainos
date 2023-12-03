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
"╭         ╭──A──╮                         ╭─────╮                                         ╭─────╮ ╭─────╮",
"├─────────┤     ├──── ─────── ────────────┤     ├─────────────────────────────────────────┤     ├─┤     ├────╮",
"╰         ╰09─10╯    │       │            ╰─────╯                                         ╰─────╯ ╰─────╯    │",
"                     │       │                                                                               │",
"╭         ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮    │",
"├─────────┤     ├───  16   15  ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├───",
"╰         ╰─────╯    │       │            ╰─────╯         ╰─────╯         ╰─────╯         ╰─────╯ ╰─────╯    │",
"                     │       │                       │                               │                       │",
"╭         ╭─────╮    │    ╭──┴──╮                 ╭──┴──╮ ╭─────╮         ╭─────╮ ╭──┴──╮                    │",
"├─────────┤     ├────╯    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                    │",
"╰         ╰─────╯         ╰──┬──╯                 ╰─────╯ ╰─────╯    │    ╰─────╯ ╰─────╯                    │",
"                             │                                       │                                       │",
"╭ ╭─────╮                 ╭──┴──╮                 ╭─────╮ ╭─────╮    │    ╭─────╮ ╭─────╮                    │",
"├─┤     ├────────────╮    │     │                 │     ├─┤     ├──── ────┤     ├─┤     │                    │",
"╰ ╰─────╯            │    ╰──┬──╯                 ╰──┬──╯ ╰─────╯         ╰─────╯ ╰──┬──╯                    │",
"                     │       │                       │                               │                       │",
"╭ ╭─────╮ ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮    │",
"├─┤     ├─┤     ├───           ───────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├───",
"╰ ╰─────╯ ╰─────╯    │       │            ╰─────╯         ╰─────╯         ╰─────╯         ╰─────╯ ╰─────╯    │",
"                     │       │                                                                               │",
"╭ ╭─────╮ ╭─────╮    │       │            ╭─────╮         ╭─────╮         ╭─────╮         ╭─────╮ ╭─────╮    │",
"├─┤     ├─┤     ├───         ╰────────────┤     ├──── ────┤     ├─────────┤     ├──── ────┤     ├─┤     ├────╯",
"╰ ╰─────╯ ╰─────╯    │                    ╰─────╯    │    ╰─────╯         ╰─────╯    │    ╰─────╯ ╰─────╯",
"                     │                               │                               │",
"╭ ╭─────╮ ╭─────╮    │                    ╭─────╮    │                               │    ╭─────╮            ╮",
"├─┤     ├─┤     ├──── ────────────────────┤     ├──── ─────────────────────────────── ────┤     ├────────────┤",
"╰ ╰─────╯ ╰─────╯                         ╰─────╯                                         ╰─────╯            ╯",
0
};

#endif // __UI_DIAGRAM_H__
