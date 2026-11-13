// 16 blocks 2KB apart: they all map to the same set in L1, L2 and L3,
// forcing evictions (and write backs in write back configurations)
// EXPECT r5 = 272
// EXPECT mem32[0x40000] = 1
// EXPECT mem32[0x47800] = 16
// EXPECT mem32[0x47804] = 16

        lui     r10, 0x40
        lui     r6, 0x1
        addi    r6, r6, -2048       // stride 0x800
        addi    r1, r0, 0
        addi    r2, r0, 16
        add     r3, r10, r0
wloop:
        addi    r4, r1, 1
        sw      r4, 0(r3)
        sw      r4, 4(r3)
        add     r3, r3, r6
        addi    r1, r1, 1
        blt     r1, r2, wloop
        addi    r1, r0, 0
        add     r3, r10, r0
        addi    r5, r0, 0
rloop:
        lw      r4, 0(r3)
        add     r5, r5, r4
        lw      r4, 4(r3)
        add     r5, r5, r4
        add     r3, r3, r6
        addi    r1, r1, 1
        blt     r1, r2, rloop

        addi    r17, r0, 0
        ecall
