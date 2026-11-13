// fence and nop execute as bubbles
// EXPECT r1 = 3

        addi    r1, r0, 1
        fence
        addi    r1, r1, 1
        nop
        addi    r1, r1, 1
        addi    r17, r0, 0
        ecall
