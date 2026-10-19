// Copied from collegereport/demo/asm/04_array.s (the documented demo program).
// EXPECT r5 = 2016
// EXPECT r6 = 2016
// EXPECT mem32[0x40000] = 0
// EXPECT mem32[0x400fc] = 63

// ============================================================
// 04_array.s  -  array fill + two passes of reads (cache demo)
//
// Writes a[i] = i for i = 0..63 (64 words = 256 bytes) at
// 0x40000, then sums the array TWICE.
//   pass 1: cold misses (one miss per cache block)
//   pass 2: everything hits, because 256 B fits in the L1-D
// Compare the L1 D hit/miss counters between configurations.
//
// Expected:
//   r5 = 000007e0 (2016 = 0+1+...+63)  after pass 1
//   r6 = 000007e0 (2016)               after pass 2
// ============================================================

        lui     r10, 0x40          // base = 0x40000
        addi    r3, r0, 64         // n = 64

// ---- fill: a[i] = i ----
        addi    r1, r0, 0          // i = 0
        addi    r2, r10, 0         // p = base
fill:
        sw      r1, 0(r2)
        addi    r1, r1, 1
        addi    r2, r2, 4
        bne     r1, r3, fill

// ---- pass 1: sum into r5 ----
        addi    r5, r0, 0
        addi    r1, r0, 0
        addi    r2, r10, 0
pass1:
        lw      r4, 0(r2)
        add     r5, r5, r4
        addi    r1, r1, 1
        addi    r2, r2, 4
        bne     r1, r3, pass1

// ---- pass 2: same sum into r6 ----
        addi    r6, r0, 0
        addi    r1, r0, 0
        addi    r2, r10, 0
pass2:
        lw      r4, 0(r2)
        add     r6, r6, r4
        addi    r1, r1, 1
        addi    r2, r2, 4
        bne     r1, r3, pass2

        addi    r17, r0, 0
        ecall
