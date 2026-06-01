// ============================================================
// BINARY SEARCH
//
// Search 11 in:
// [1,3,5,7,9,11,13,15]
//
// Expected:
// 0x0900 = 5
// ============================================================


// ------------------------------------------------------------
// Initialize array
// ------------------------------------------------------------

        lui     r1, 0x00001
        addi    r1, r1, -2048          // 0x0800

        addi    r2, r0, 1
        sw      r2, 0(r1)

        addi    r2, r0, 3
        sw      r2, 4(r1)

        addi    r2, r0, 5
        sw      r2, 8(r1)

        addi    r2, r0, 7
        sw      r2, 12(r1)

        addi    r2, r0, 9
        sw      r2, 16(r1)

        addi    r2, r0, 11
        sw      r2, 20(r1)

        addi    r2, r0, 13
        sw      r2, 24(r1)

        addi    r2, r0, 15
        sw      r2, 28(r1)


// ------------------------------------------------------------
// low = 0
// high = 7
// target = 11
// ------------------------------------------------------------

        addi    r3, r0, 0              // low
        addi    r4, r0, 7              // high
        addi    r5, r0, 11             // target


search_loop:

// if(high < low) fail

        blt     r4, r3, not_found


// mid = (low + high) >> 1

        add     r6, r3, r4
        srli    r6, r6, 1


// arr[mid]

        slli    r7, r6, 2
        add     r7, r1, r7
        lw      r8, 0(r7)


// found?

        beq     r8, r5, found


// arr[mid] < target ?

        blt     r8, r5, go_right


// high = mid - 1

        addi    r4, r6, -1
        jal     r0, search_loop


go_right:

// low = mid + 1

        addi    r3, r6, 1
        jal     r0, search_loop


found:

// store index

        lui     r9, 0x00001
        addi    r9, r9, -1792          // 0x0900

        sw      r6, 0(r9)

        jal     r0, done


not_found:

        lui     r9, 0x00001
        addi    r9, r9, -1792

        addi    r10, r0, -1
        sw      r10, 0(r9)


done:

        ebreak
        nop
        nop
        nop
        nop
