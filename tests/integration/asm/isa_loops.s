// 1KB array fill + sum (more data than the tiny caches hold)
// EXPECT r4 = 32640
// EXPECT mem32[0x40400] = 32640
// EXPECT mem32[0x403fc] = 255
// EXPECT mem32[0x40000] = 0

        lui     r10, 0x40
        addi    r1, r0, 0
        addi    r2, r0, 256
fill:
        slli    r3, r1, 2
        add     r3, r3, r10
        sw      r1, 0(r3)
        addi    r1, r1, 1
        blt     r1, r2, fill
        addi    r1, r0, 0
        addi    r4, r0, 0
sum:
        slli    r3, r1, 2
        add     r3, r3, r10
        lw      r5, 0(r3)
        add     r4, r4, r5
        addi    r1, r1, 1
        bne     r1, r2, sum
        sw      r4, 1024(r10)

        addi    r17, r0, 0
        ecall
