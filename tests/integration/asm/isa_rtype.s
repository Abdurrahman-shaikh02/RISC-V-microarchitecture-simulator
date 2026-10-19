// R-type instructions with edge values (results checked under every config)
// EXPECT r1 = 0xffffffff
// EXPECT r5 = 0
// EXPECT r6 = 2
// EXPECT r7 = 0x7fffffff
// EXPECT r8 = 0x80000000
// EXPECT r9 = 1
// EXPECT r11 = 0xffffffff
// EXPECT r12 = 1
// EXPECT r13 = 0
// EXPECT r14 = 1
// EXPECT r15 = 0
// EXPECT r16 = 0x7fffffff
// EXPECT r18 = 0x80000001
// EXPECT r19 = 0x80000000
// EXPECT r21 = 2
// EXPECT r0 = 0
// EXPECT r22 = 0

        addi    r1, r0, -1          // 0xffffffff
        addi    r2, r0, 1
        lui     r4, 0x80000         // 0x80000000
        add     r5, r1, r2          // wraps to 0
        sub     r6, r2, r1          // 1 - (-1) = 2
        sub     r7, r4, r2          // 0x7fffffff
        sll     r8, r2, r1          // shamt = 31
        srl     r9, r4, r1          // 1
        sra     r11, r4, r1         // sign fill
        slt     r12, r1, r2         // -1 < 1
        sltu    r13, r1, r2         // 0xffffffff < 1 is false
        slt     r14, r4, r7         // INT_MIN < INT_MAX
        sltu    r15, r4, r7         // 0x80000000 < 0x7fffffff is false
        xor     r16, r1, r4
        or      r18, r4, r2
        and     r19, r1, r4
        addi    r20, r0, 33
        sll     r21, r2, r20        // only 5 bits of the shift amount: 1 << 1
        add     r0, r1, r1          // writes to x0 are discarded
        add     r22, r0, r0

        addi    r17, r0, 0
        ecall
