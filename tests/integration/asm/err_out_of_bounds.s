// the test configs use 1MB of DRAM
// EXPECT exit = 1
// EXPECT output: [FATAL]: Out of bounds memory read.
        lui     r2, 0x100
        lw      r1, 0(r2)
        addi    r17, r0, 0
        ecall
