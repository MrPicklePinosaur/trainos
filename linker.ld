ENTRY(kmain)
SECTIONS {
  . = 0x200000;          /* start text at this location */
  .text : {              /* force kmain to start of text section, with all other functions following */
    *(.text.kmain);
    *(.text*)
  }
}
