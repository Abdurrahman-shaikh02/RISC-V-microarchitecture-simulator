.section .text.startup
.global _start
_start:
    la sp, __stack_top
    call main
    la t0, tohost
    slli a0, a0, 1
    ori a0, a0, 1
    sw a0, 0(t0)
    sw zero, 4(t0)
1:  j 1b
.section .tohost,"aw",@progbits
.align 6
.globl tohost
tohost: .dword 0
.align 6
.globl fromhost
fromhost: .dword 0
