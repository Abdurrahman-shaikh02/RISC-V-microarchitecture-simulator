/* linker.ld */

ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x00000000, LENGTH = 1024M
}

SECTIONS
{
    . = ORIGIN(RAM);

    /* Startup code first */
    .text :
    {
        KEEP(*(.text.startup))
        KEEP(*(.text._start))
        *(.text)
        *(.text.*)
    } > RAM

    . = 0x00020000;     /*128kb for code*/

    .rodata ALIGN(4) :
    {
        *(.rodata)
        *(.rodata.*)
    } > RAM

    . = 0x00040000;     /*128kb for read only data*/

    .data ALIGN(4) :
    {
        __data_start = .;

        *(.data)
        *(.data.*)

        __data_end = .;
    } > RAM

    .bss ALIGN(4) :
    {
        __bss_start = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        __bss_end = .;
    } > RAM

    /* End of program image */
    __program_end = .;
    _end = .;

    /* Heap starts immediately after the program */
    __heap_start = .;

    /* Stack grows downward from the top of RAM */
    __stack_top = ORIGIN(RAM) + LENGTH(RAM);

    /* Heap must never grow past this */
    __heap_end = __stack_top;
}
