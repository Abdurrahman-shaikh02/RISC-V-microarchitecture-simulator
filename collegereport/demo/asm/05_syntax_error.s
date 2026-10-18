// ============================================================
// 05_syntax_error.s  -  shows the assembler rejecting bad input
//
// Line 10 uses an instruction that does not exist in RV32I
// (RV32I has no 'mul'; that is the M extension). The assembler
// should stop with "Invalid operation name" and the line number.
// ============================================================

        addi    r1, r0, 6
        mul     r2, r1, r1         // <- not an RV32I instruction
        addi    r17, r0, 0
        ecall
