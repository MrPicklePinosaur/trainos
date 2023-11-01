ENTRY(_start)
SECTIONS {
  . = 0x80000;            /* start text at this location */
  .text.boot : {          /* boot code must come first */
    KEEP(*(.text.boot))
  }
}
