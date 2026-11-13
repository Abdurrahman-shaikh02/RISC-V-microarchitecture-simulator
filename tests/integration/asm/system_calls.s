// ecall with a7 != 0 is ignored, a7 == 0 halts; everything before the
// halting ecall completes (including memory operations), nothing after it runs
// EXPECT r1 = 1
// EXPECT r2 = 2
// EXPECT r3 = 0
// EXPECT r4 = 0x40000
// EXPECT mem32[0x40000] = 0x40000

        addi    r17, r0, 5
        addi    r1, r0, 1
        ecall                       // not a halt
        addi    r2, r0, 2
        lui     r10, 0x40
        sw      r10, 0(r10)
        lw      r4, 0(r10)
        addi    r17, r0, 0
        ecall                       // halt
        addi    r3, r0, 3
        addi    r3, r0, 3
