// ============================================================
// SELECTION SORT
//
// Initial:
// 7, -3, 15, 0, -8, 4
//
// Expected: at 0x800
// -8, -3, 0, 4, 7, 15
// ============================================================


// ------------------------------------------------------------
// Initialize Array
// ------------------------------------------------------------

        lui     r1, 0x00001
        addi    r1, r1, -2048          // 0x0800

        addi    r2, r0, 7
        sw      r2, 0(r1)

        addi    r2, r0, -3
        sw      r2, 4(r1)

        addi    r2, r0, 15
        sw      r2, 8(r1)

        addi    r2, r0, 0
        sw      r2, 12(r1)

        addi    r2, r0, -8
        sw      r2, 16(r1)

        addi    r2, r0, 4
        sw      r2, 20(r1)

        addi    r2, r0, 6              // length


// ------------------------------------------------------------
// i = 0
// ------------------------------------------------------------

        addi    r3, r0, 0              // i

outer_loop:

        addi    r4, r2, -1             // n-1

        bge     r3, r4, done

        addi    r5, r3, 0              // min = i

        addi    r6, r3, 1              // j = i+1


inner_loop:

        bge     r6, r2, do_swap


// arr[j]

        slli    r7, r6, 2
        add     r7, r1, r7
        lw      r8, 0(r7)


// arr[min]

        slli    r9, r5, 2
        add     r9, r1, r9
        lw      r10, 0(r9)


// if(arr[j] < arr[min])

        bge     r8, r10, next_j

        addi    r5, r6, 0


next_j:

        addi    r6, r6, 1

        jal     r0, inner_loop



// ------------------------------------------------------------
// swap arr[i] and arr[min]
// ------------------------------------------------------------

do_swap:

        slli    r7, r3, 2
        add     r7, r1, r7
        lw      r8, 0(r7)

        slli    r9, r5, 2
        add     r9, r1, r9
        lw      r10, 0(r9)

        sw      r10, 0(r7)
        sw      r8, 0(r9)

        addi    r3, r3, 1

        jal     r0, outer_loop


done:

        ebreak
        nop
        nop
        nop
        nop
