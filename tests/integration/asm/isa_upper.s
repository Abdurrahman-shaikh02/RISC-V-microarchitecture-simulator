// lui / auipc and building 32 bit constants
// EXPECT r1 = 0x12345000
// EXPECT r2 = 0xfffff000
// EXPECT r3 = 8
// EXPECT r4 = 0x100c
// EXPECT r5 = 0x3f900
// EXPECT r6 = 0xfff
// EXPECT r7 = 0x12345678

        lui     r1, 0x12345
        lui     r2, 0xfffff
        auipc   r3, 0               // address of this instruction (8)
        auipc   r4, 1               // 0x1000 + 12
        lui     r5, 0x40
        addi    r5, r5, -1792       // 0x40000 - 0x700
        lui     r6, 0x1
        addi    r6, r6, -1
        lui     r7, 0x12345
        addi    r7, r7, 0x678

        addi    r17, r0, 0
        ecall
