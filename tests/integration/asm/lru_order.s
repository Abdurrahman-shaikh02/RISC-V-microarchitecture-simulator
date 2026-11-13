// A, B, A, C, A in one 2-way L1-D set (1KB apart): with LRU the hit on A
// protects it and C evicts B, with FIFO C evicts A. The timing golden of the
// LRU configurations depends on the hit updating the LRU order.
// EXPECT r5 = 6
        lui     r10, 0x40
        lui     r11, 0x1
        addi    r11, r11, -1024     // 0x400
        add     r12, r10, r11       // B = A + 0x400
        add     r13, r12, r11       // C = A + 0x800
        addi    r1, r0, 1
        sw      r1, 0(r10)
        addi    r1, r0, 2
        sw      r1, 0(r12)
        addi    r1, r0, 3
        sw      r1, 0(r13)
        addi    r5, r0, 0
        lw      r2, 0(r10)          // A (filled by the stores above, order A B C)
        lw      r3, 0(r12)          // B
        lw      r2, 0(r10)          // A again: hit, A becomes most recent
        lw      r4, 0(r13)          // C: evicts the least recent
        lw      r2, 0(r10)          // A: hit only with LRU
        add     r5, r2, r3
        add     r5, r5, r4
        addi    r17, r0, 0
        ecall
