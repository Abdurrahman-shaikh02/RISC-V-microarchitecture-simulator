// Copied from collegereport/demo/asm/02_sum_loop.s (the documented demo program).
// EXPECT r1 = 55
// EXPECT mem32[0x40000] = 0x37

// ============================================================
// 02_sum_loop.s  -  loop with a backward branch + store
//
// sum = 1 + 2 + ... + 10 = 55, stored to memory at 0x40000
// (the first address after the text and rodata segments).
//
// The loop-closing 'bne' is a BACKWARD branch taken 9 times
// and not taken once, so it is a good branch-prediction demo:
//   not-taken predictor : 9 mispredictions
//   taken / BTFN        : 1 misprediction
//
// Expected:
//   r1 = 00000037 (55)   in register_dump
//   word at 0x40000 = 0x37   in memory_dump.bin
// ============================================================

        addi    r1, r0, 0          // sum = 0
        addi    r2, r0, 1          // i   = 1
        addi    r3, r0, 11         // limit = 11

loop:
        add     r1, r1, r2         // sum += i
        addi    r2, r2, 1          // i++
        bne     r2, r3, loop       // while (i != 11)

        lui     r10, 0x40          // r10 = 0x00040000
        sw      r1, 0(r10)         // mem[0x40000] = sum

        addi    r17, r0, 0
        ecall
