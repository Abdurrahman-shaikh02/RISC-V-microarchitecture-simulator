// ebreak stops the simulation (statistics are still printed)
// EXPECT r1 = 1
// EXPECT r2 = 0
// EXPECT output: Generating Stats...

        addi    r1, r0, 1
        ebreak
        addi    r2, r0, 2
        addi    r2, r0, 2
