// recursive fib(10) with a stack at the top of the 1MB test memory
// EXPECT r11 = 55
// EXPECT r2 = 0x100000

        lui     r2, 0x100           // sp = 1MB
        addi    r10, r0, 10
        jal     r1, fib
        addi    r11, r10, 0
        addi    r17, r0, 0
        ecall

fib:
        addi    r5, r0, 2
        blt     r10, r5, fib_base
        addi    r2, r2, -12
        sw      r1, 8(r2)
        sw      r8, 4(r2)
        sw      r10, 0(r2)
        addi    r10, r10, -1
        jal     r1, fib
        addi    r8, r10, 0          // fib(n - 1)
        lw      r10, 0(r2)
        addi    r10, r10, -2
        jal     r1, fib
        add     r10, r10, r8
        lw      r8, 4(r2)
        lw      r1, 8(r2)
        addi    r2, r2, 12
        jalr    r0, 0(r1)
fib_base:
        jalr    r0, 0(r1)
