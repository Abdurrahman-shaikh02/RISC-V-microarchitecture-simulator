// ==========================
// BASIC SETUP
// ==========================

addi r1, r0, 10
nop
nop

addi r2, r0, 3
nop
nop

addi r10, r0, -8
nop
nop

// ==========================
// R-TYPE TESTS
// ==========================

// add : 10 + 3 = 13
add r3, r1, r2
nop
nop

// sub : 10 - 3 = 7
sub r4, r1, r2
nop
nop

// sll : 10 << 3 = 80
sll r5, r1, r2
nop
nop

// slt : 3 < 10 = 1
slt r6, r2, r1
nop
nop

// sltu : 3 < 10 = 1
sltu r7, r2, r1
nop
nop

// xor : 10 ^ 3 = 9
xor r8, r1, r2
nop
nop

// srl : 10 >> 3 = 1
srl r9, r1, r2
nop
nop

// sra : -8 >> 3 = -1
sra r11, r10, r2
nop
nop

// or : 10 | 3 = 11
or r12, r1, r2
nop
nop

// and : 10 & 3 = 2
and r13, r1, r2
nop
nop

// ==========================
// I-TYPE TESTS
// ==========================

// addi : 10 + (-5) = 5
addi r14, r1, -5
nop
nop

// slti : 3 < 5 = 1
slti r15, r2, 5
nop
nop

// sltiu : 3 < 20 = 1
sltiu r16, r2, 20
nop
nop

// xori : 10 ^ 6 = 12
xori r17, r1, 6
nop
nop

// ori : 10 | 4 = 14
ori r18, r1, 4
nop
nop

// andi : 10 & 6 = 2
andi r19, r1, 6
nop
nop

// ==========================
// SHIFT-IMM TESTS
// ==========================

// slli : 10 << 2 = 40
slli r20, r1, 2
nop
nop

// srli : 10 >> 1 = 5
srli r21, r1, 1
nop
nop

// srai : -8 >> 2 = -2
srai r22, r10, 2
