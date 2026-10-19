.section .text.startup
.global _start
.extern main
.extern __stack_top

_start:
    /* Initialize stack pointer */
    la      sp, __stack_top

    /* Call C program */
    call    main

    /* Stop simulator */
    xor a7, a7, a7
    ecall
