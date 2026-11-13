// I-type ALU instructions, sign extension of immediates
// EXPECT r1 = 100
// EXPECT r2 = 0xffffff9c
// EXPECT r3 = 2047
// EXPECT r4 = 0xfffff800
// EXPECT r5 = 1
// EXPECT r6 = 0
// EXPECT r7 = 1
// EXPECT r8 = 0
// EXPECT r9 = 1
// EXPECT r11 = 0xffffff9b
// EXPECT r12 = 0xfffff800
// EXPECT r13 = 0x6f
// EXPECT r14 = 0x9c
// EXPECT r15 = 0xffffff90
// EXPECT r16 = 0x64000000
// EXPECT r18 = 0xf
// EXPECT r19 = 0xfffffff9
// EXPECT r20 = 0xffffffff
// EXPECT r21 = 1
// EXPECT r22 = 2047
// EXPECT r23 = -16

        addi    r1, r0, 100
        addi    r2, r1, -200        // -100
        addi    r3, r0, 2047        // largest immediate
        addi    r4, r0, -2048       // smallest immediate
        slti    r5, r2, -99         // -100 < -99
        slti    r6, r1, -99
        sltiu   r7, r1, -1          // compared with 0xffffffff
        sltiu   r8, r2, 100
        sltiu   r9, r0, 1           // seqz idiom
        xori    r11, r1, -1         // not idiom
        ori     r12, r0, -2048
        ori     r13, r1, 0x0f
        andi    r14, r2, 0xff
        andi    r15, r2, -16
        slli    r16, r1, 24
        srli    r18, r2, 28
        srai    r19, r2, 4          // -100 >> 4 = -7
        srai    r20, r4, 11
        srli    r21, r4, 31
        addi    r22, r0, 0x7ff      // hex immediate
        addi    r23, r0, -0x10      // negative hex immediate

        addi    r17, r0, 0
        ecall
