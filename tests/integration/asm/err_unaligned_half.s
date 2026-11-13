// EXPECT exit = 1
// EXPECT output: [FATAL]: Unaligned halfword access
        lui     r2, 0x40
        sh      r1, 1(r2)
        addi    r17, r0, 0
        ecall
