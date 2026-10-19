// every branch, taken and not taken, signed and unsigned
// r21 counts wrong paths (must stay 0), r22 counts correct paths
// EXPECT r21 = 0
// EXPECT r22 = 18
// EXPECT r23 = 5

        addi    r1, r0, -1
        addi    r2, r0, 1
        addi    r3, r0, 1
        addi    r21, r0, 0
        addi    r22, r0, 0

// ---- taken ----
        beq     r2, r3, t1
        addi    r21, r21, 1
t1:
        addi    r22, r22, 1
        bne     r1, r2, t2
        addi    r21, r21, 1
t2:
        addi    r22, r22, 1
        blt     r1, r2, t3          // -1 < 1 signed
        addi    r21, r21, 1
t3:
        addi    r22, r22, 1
        bge     r2, r1, t4          // 1 >= -1 signed
        addi    r21, r21, 1
t4:
        addi    r22, r22, 1
        bge     r2, r3, t5          // equal
        addi    r21, r21, 1
t5:
        addi    r22, r22, 1
        bltu    r2, r1, t6          // 1 < 0xffffffff unsigned
        addi    r21, r21, 1
t6:
        addi    r22, r22, 1
        bgeu    r1, r2, t7
        addi    r21, r21, 1
t7:
        addi    r22, r22, 1
        bgeu    r2, r3, t8          // equal
        addi    r21, r21, 1
t8:
        addi    r22, r22, 1
        beq     r0, r0, t9
        addi    r21, r21, 1
t9:
        addi    r22, r22, 1

// ---- not taken ----
        beq     r1, r2, bad
        addi    r22, r22, 1
        bne     r2, r3, bad
        addi    r22, r22, 1
        blt     r2, r1, bad         // 1 < -1 signed
        addi    r22, r22, 1
        blt     r2, r3, bad         // equal
        addi    r22, r22, 1
        bge     r1, r2, bad
        addi    r22, r22, 1
        bltu    r1, r2, bad         // 0xffffffff < 1 unsigned
        addi    r22, r22, 1
        bltu    r2, r3, bad         // equal
        addi    r22, r22, 1
        bgeu    r2, r1, bad
        addi    r22, r22, 1
        bne     r0, r0, bad
        addi    r22, r22, 1

// ---- backward branch loop ----
        addi    r23, r0, 0
        addi    r24, r0, 5
loop:
        addi    r23, r23, 1
        bne     r23, r24, loop

        addi    r17, r0, 0
        ecall

bad:
        addi    r21, r21, 100
        addi    r17, r0, 0
        ecall
