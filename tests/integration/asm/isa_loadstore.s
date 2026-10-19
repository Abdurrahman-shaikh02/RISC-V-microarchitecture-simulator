// loads and stores of every width, sign/zero extension, little endian layout
// EXPECT r1 = 0x81828384
// EXPECT r2 = 0x81828384
// EXPECT r3 = 0xffffff84
// EXPECT r4 = 0x84
// EXPECT r5 = 0xffffff83
// EXPECT r6 = 0xffff8384
// EXPECT r7 = 0x8182
// EXPECT r8 = 0xffff8182
// EXPECT r11 = 0x8384847f
// EXPECT r12 = 0x7f
// EXPECT r14 = 0xffff00ff
// EXPECT r15 = 0x000000ff
// EXPECT r18 = 0x81828384
// EXPECT r19 = 0x00040537
// EXPECT mem32[0x40000] = 0x81828384
// EXPECT mem32[0x40004] = 0x8384847f
// EXPECT mem32[0x40008] = 0x000000ff
// EXPECT mem32[0x4003c] = 0x81828384
// EXPECT mem8[0x40005] = 0x84

        lui     r10, 0x40           // data base 0x40000
        lui     r1, 0x81828
        addi    r1, r1, 0x384       // 0x81828384
        sw      r1, 0(r10)
        lw      r2, 0(r10)
        lb      r3, 0(r10)          // 0x84 sign extended
        lbu     r4, 0(r10)
        lb      r5, 1(r10)          // 0x83
        lh      r6, 0(r10)          // 0x8384
        lhu     r7, 2(r10)          // 0x8182
        lh      r8, 2(r10)
        addi    r9, r0, 0x7f
        sb      r9, 4(r10)
        sb      r1, 5(r10)          // low byte only
        sh      r1, 6(r10)          // low half only
        lw      r11, 4(r10)
        lb      r12, 4(r10)         // positive byte
        addi    r13, r0, -1
        sw      r13, 8(r10)
        sb      r0, 9(r10)
        lw      r14, 8(r10)
        sh      r0, 10(r10)
        lw      r15, 8(r10)
        addi    r16, r10, 64
        sw      r1, -4(r16)         // negative offset
        lw      r18, -4(r16)
        lw      r19, 0(r0)          // the text segment is readable: first instruction

        addi    r17, r0, 0
        ecall
