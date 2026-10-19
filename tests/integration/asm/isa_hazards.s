// RAW dependencies at every distance, load-use, store data, x0, branch on load
// EXPECT r2 = 6
// EXPECT r3 = 7
// EXPECT r4 = 12
// EXPECT r5 = 24
// EXPECT r6 = 29
// EXPECT r7 = 29
// EXPECT r8 = 58
// EXPECT r11 = 29
// EXPECT r12 = 30
// EXPECT r14 = 30
// EXPECT r15 = 4
// EXPECT r16 = 4
// EXPECT r19 = 0
// EXPECT r20 = 1
// EXPECT r22 = 4
// EXPECT r23 = 140
// EXPECT r24 = 140
// EXPECT mem32[0x40004] = 29
// EXPECT mem32[0x40008] = 30
// EXPECT mem32[0x4000c] = 30
// EXPECT mem32[0x40010] = 4

        addi    r1, r0, 5
        addi    r2, r1, 1           // distance 1
        addi    r3, r2, 1
        add     r4, r3, r1          // distances 1 and 3
        nop
        add     r5, r4, r4          // distance 2
        nop
        nop
        add     r6, r5, r1          // distance 3
        lui     r10, 0x40
        sw      r6, 0(r10)
        lw      r7, 0(r10)
        add     r8, r7, r7          // load-use
        lw      r9, 0(r10)
        sw      r9, 4(r10)          // loaded value as store data
        lw      r11, 4(r10)
        addi    r12, r11, 1         // load-use
        sw      r12, 8(r10)         // ALU result as store data
        lw      r13, 8(r10)
        sw      r13, 12(r10)
        lw      r14, 12(r10)
        addi    r15, r0, 1
        addi    r15, r15, 1
        addi    r15, r15, 1
        addi    r15, r15, 1
        add     r0, r15, r15        // must not be forwarded as x0
        add     r16, r0, r15
        lw      r18, 0(r10)
        beq     r18, r6, eq         // branch on a just loaded value
        addi    r19, r0, 99
eq:
        addi    r20, r0, 1
        addi    r21, r10, 16
        sw      r15, 0(r21)         // base register just computed
        lw      r22, 0(r21)
        jal     r23, next           // 136: link used right away
next:
        addi    r24, r23, 0

        addi    r17, r0, 0
        ecall
