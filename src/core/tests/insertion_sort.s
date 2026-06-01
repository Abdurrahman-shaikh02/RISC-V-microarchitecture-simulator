// ============================================================
// RV32I INSERTION SORT
// Array base : 0x0800
// Length     : 0x0818
//
// Initial Array:
// 25, -7, 18, 3, -42, 11
//
// Result:
// -42, -7, 3, 11, 18, 25
// ============================================================


// ============================================================
// MEMORY INITIALIZATION
// ============================================================
        lui     r1, 0x00001
        addi    r1, r1, -2048          // r1 = 0x0800

        addi    r2, r0, 2047
        sw      r2, 0(r1)

        addi    r2, r0, -2048
        sw      r2, 4(r1)

        addi    r2, r0, 0
        sw      r2, 8(r1)

        addi    r2, r0, -1
        sw      r2, 12(r1)

        addi    r2, r0, 1
        sw      r2, 16(r1)

        addi    r2, r0, 1024
        sw      r2, 20(r1)

        addi    r2, r0, 6
        sw      r2, 24(r1)
// ============================================================
// INSERTION SORT
// ============================================================

// r1 = base address
// r2 = length
// r3 = i
// r4 = key
// r5 = j
// r6 = arr[j]
// r7 = scratch
// r8 = address of arr[j]
// r9 = address of arr[j+1]

        lw      r2, 24(r1)             // length

        addi    r3, r0, 1              // i = 1

outer_loop:

        bge     r3, r2, done

// key = arr[i]

        slli    r7, r3, 2
        add     r7, r1, r7
        lw      r4, 0(r7)

        addi    r5, r3, -1             // j = i - 1

inner_loop:

// if j < 0 -> insert key

        blt     r5, r0, insert_key

// load arr[j]

        slli    r8, r5, 2
        add     r8, r1, r8
        lw      r6, 0(r8)

// if arr[j] <= key -> insert

        bge     r4, r6, insert_key

// arr[j+1] = arr[j]

        addi    r9, r8, 4
        sw      r6, 0(r9)

// j--

        addi    r5, r5, -1

        jal     r0, inner_loop


insert_key:

// j++

        addi    r5, r5, 1

// arr[j+1] location becomes arr[j]

        slli    r9, r5, 2
        add     r9, r1, r9

// store key

        sw      r4, 0(r9)

// i++

        addi    r3, r3, 1

        jal     r0, outer_loop


done:

        ebreak
        nop
        nop
        nop
        nop
