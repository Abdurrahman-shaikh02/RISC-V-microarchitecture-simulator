// -----------------------------------------------------
// multiply
//
// Inputs:
//     r14 = multiplicand
//     r15 = multiplier
//
// Output:
//     r19 = product
//
// Destroys:
//     r20
// -----------------------------------------------------

multiply:

    addi r19, r0, 0          // product = 0

mul_loop:

    beq r15, r0, mul_done    // while(multiplier != 0)

    andi r20, r15, 1         // check LSB

    beq r20, r0, no_add

    add r19, r19, r14        // product += multiplicand

no_add:

    slli r14, r14, 1         // multiplicand <<= 1

    srli r15, r15, 1         // multiplier >>= 1

    jal r0, mul_loop

mul_done:

    // control comes here after completion
