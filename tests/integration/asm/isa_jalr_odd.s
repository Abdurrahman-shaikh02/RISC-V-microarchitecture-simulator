// jalr must clear bit 0 of the target address (rs1 + imm) & ~1
// XFAIL: jalr does not clear bit 0 of the target, the next fetch is unaligned
// EXPECT r5 = 0
// EXPECT r6 = 1
// CONFIGS: ideal

        addi    r1, r0, 13          // 0
        jalr    r0, 0(r1)           // 4: target 13 & ~1 = 12
        addi    r5, r0, 99          // 8: skipped
        addi    r6, r0, 1           // 12
        addi    r17, r0, 0
        ecall
