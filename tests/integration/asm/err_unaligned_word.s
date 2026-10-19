// EXPECT exit = 1
// EXPECT output: [FATAL]: Unaligned word access
        lui     r2, 0x40
        lw      r1, 2(r2)
        addi    r17, r0, 0
        ecall
