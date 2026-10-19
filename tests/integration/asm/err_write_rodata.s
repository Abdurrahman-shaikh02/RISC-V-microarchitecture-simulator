// EXPECT exit = 1
// EXPECT output: [FATAL]: attempt to write to rodata segment
        lui     r2, 0x20
        sw      r1, 0(r2)
        addi    r17, r0, 0
        ecall
