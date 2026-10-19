// EXPECT exit = 1
// EXPECT output: [FATAL]: attempt to write to text segment
        addi    r1, r0, 1
        sw      r1, 256(r0)
        addi    r17, r0, 0
        ecall
