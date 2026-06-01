// ============================================================
// BUBBLE SORT
//
// Initial:
// 9, -2, 7, 0, -5, 3
//
// Expected: at 0x800
// -5, -2, 0, 3, 7, 9
// ============================================================


// ------------------------------------------------------------
// Initialize array
// ------------------------------------------------------------

        lui     r1, 0x00001
        addi    r1, r1, -2048

        addi    r2, r0, 9
        sw      r2, 0(r1)

        addi    r2, r0, -2
        sw      r2, 4(r1)

        addi    r2, r0, 7
        sw      r2, 8(r1)

        addi    r2, r0, 0
        sw      r2, 12(r1)

        addi    r2, r0, -5
        sw      r2, 16(r1)

        addi    r2, r0, 3
        sw      r2, 20(r1)

        addi    r2, r0, 6              // n


// ------------------------------------------------------------
// i = 0
// ------------------------------------------------------------

        addi    r3, r0, 0


outer_loop:

        addi    r4, r2, -1

        bge     r3, r4, done

        addi    r5, r0, 0              // j = 0


inner_loop:

// limit = n - i - 1

        sub     r6, r2, r3
        addi    r6, r6, -1

        bge     r5, r6, next_outer


// arr[j]

        slli    r7, r5, 2
        add     r7, r1, r7
        lw      r8, 0(r7)


// arr[j+1]

        addi    r9, r7, 4
        lw      r10, 0(r9)


// if arr[j] <= arr[j+1] skip swap

        bge     r10, r8, no_swap


// swap

        sw      r10, 0(r7)
        sw      r8, 0(r9)


no_swap:

        addi    r5, r5, 1

        jal     r0, inner_loop


next_outer:

        addi    r3, r3, 1

        jal     r0, outer_loop


done:

        ebreak
        nop
        nop
        nop
        nop
