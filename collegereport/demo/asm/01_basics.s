// ============================================================
// 01_basics.s  -  arithmetic, logic and shifts
//
// Nothing fancy: every instruction just writes a register,
// so the result is visible in register_dump after the run.
//
// Expected (register_dump):
//   r1  = 0000000c   (12)
//   r2  = 00000005   (5)
//   r3  = 00000011   (12 + 5 = 17)
//   r4  = 00000007   (12 - 5 = 7)
//   r5  = 00000004   (12 & 5)
//   r6  = 0000000d   (12 | 5)
//   r7  = 00000009   (12 ^ 5)
//   r8  = 00000060   (12 << 3 = 96)
//   r9  = 00000003   (12 >> 2)
//   r10 = fffffffe   (-2)
//   r11 = 00000001   (r10 < r1 signed   -> 1)
//   r12 = 00000000   (r10 < r1 unsigned -> 0)
//   r13 = 12345000   (lui)
// ============================================================

        addi    r1, r0, 12         // r1 = 12
        addi    r2, r0, 5          // r2 = 5

        add     r3, r1, r2         // 17
        sub     r4, r1, r2         // 7

        and     r5, r1, r2         // 4
        or      r6, r1, r2         // 13
        xor     r7, r1, r2         // 9

        slli    r8, r1, 3          // 96
        srli    r9, r1, 2          // 3

        addi    r10, r0, -2        // -2 = 0xfffffffe
        slt     r11, r10, r1       // signed:   -2 < 12  -> 1
        sltu    r12, r10, r1       // unsigned: 0xfffffffe < 12 -> 0

        lui     r13, 0x12345       // r13 = 0x12345000

// halt: ecall with a7 (r17) = 0
        addi    r17, r0, 0
        ecall
