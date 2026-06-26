// ============================================================
// ITERATIVE FACTORIAL
//
// fact(5) = 120
//
// Result:
// 0x0900 = 120 (0x78)
// ============================================================


// ------------------------------------------------------------
// n = 5
// result = 1
// i = 2
// ------------------------------------------------------------

        addi    r5, r0, 5          // n
        addi    r6, r0, 1          // result
        addi    r7, r0, 2          // i


outer_loop:

// if i > n, finished

        blt     r5, r7, done_factorial


// ------------------------------------------------------------
// result = result * i
//
// repeated addition
// ------------------------------------------------------------

        addi    r8, r0, 0          // product
        addi    r9, r7, 0          // counter = i


mul_loop:

        beq     r9, r0, mul_done

        add     r8, r8, r6

        addi    r9, r9, -1

        jal     r0, mul_loop


mul_done:

        addi    r6, r8, 0          // result = product

        addi    r7, r7, 1          // i++

        jal     r0, outer_loop



done_factorial:


// ------------------------------------------------------------
// Store result
// ------------------------------------------------------------

        lui     r10, 0x00001
        addi    r10, r10, -1792    // 0x0900

        sw      r6, 0(r10)


done:

        ebreak
        nop
        nop
        nop
        nop
