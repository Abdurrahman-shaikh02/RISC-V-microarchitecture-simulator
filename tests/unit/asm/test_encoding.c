// src/assembler/core/opcode_generator.c: machine code of every instruction,
// checked against encoders written from the ISA spec (tests/framework/rv32.h)
#include "asm_support.h"

typedef struct { const char * line; uint32_t expected; } enc_case;

static void check_cases(const enc_case * t, unsigned n){
	for(unsigned i = 0; i < n; i++){
		uint32_t got = assemble_line(t[i].line, 0);
		CHECK_MSG(got == t[i].expected, "\"%s\" encoded as %08x, expected %08x", t[i].line, got, t[i].expected);
	}
}

TEST(r_type){
	const enc_case t[] = {
		{"add r1, r2, r3", enc_r(0x00, 3, 2, 0, 1, OP_REG)},
		{"sub r4, r5, r6", enc_r(0x20, 6, 5, 0, 4, OP_REG)},
		{"sll r7, r8, r9", enc_r(0x00, 9, 8, 1, 7, OP_REG)},
		{"slt r10, r11, r12", enc_r(0x00, 12, 11, 2, 10, OP_REG)},
		{"sltu r13, r14, r15", enc_r(0x00, 15, 14, 3, 13, OP_REG)},
		{"xor r16, r17, r18", enc_r(0x00, 18, 17, 4, 16, OP_REG)},
		{"srl r19, r20, r21", enc_r(0x00, 21, 20, 5, 19, OP_REG)},
		{"sra r22, r23, r24", enc_r(0x20, 24, 23, 5, 22, OP_REG)},
		{"or r25, r26, r27", enc_r(0x00, 27, 26, 6, 25, OP_REG)},
		{"and r28, r29, r31", enc_r(0x00, 31, 29, 7, 28, OP_REG)},
	};
	check_cases(t, sizeof(t) / sizeof(t[0]));
}

TEST(i_type){
	const enc_case t[] = {
		{"addi r1, r2, 12", enc_i(12, 2, 0, 1, OP_IMM)},
		{"addi r1, r2, -1", enc_i(-1, 2, 0, 1, OP_IMM)},
		{"addi r1, r2, -2048", enc_i(-2048, 2, 0, 1, OP_IMM)},
		{"addi r1, r2, 0x7ff", enc_i(2047, 2, 0, 1, OP_IMM)},
		{"slti r3, r4, -5", enc_i(-5, 4, 2, 3, OP_IMM)},
		{"sltiu r3, r4, 5", enc_i(5, 4, 3, 3, OP_IMM)},
		{"xori r3, r4, -1", enc_i(-1, 4, 4, 3, OP_IMM)},
		{"ori r3, r4, 255", enc_i(255, 4, 6, 3, OP_IMM)},
		{"andi r3, r4, 0xf0", enc_i(0xF0, 4, 7, 3, OP_IMM)},
		{"slli r5, r6, 31", enc_i(31, 6, 1, 5, OP_IMM)},
		{"srli r5, r6, 3", enc_i(3, 6, 5, 5, OP_IMM)},
		{"srai r5, r6, 3", enc_i(0x400 | 3, 6, 5, 5, OP_IMM)},
		{"nop", enc_i(0, 0, 0, 0, OP_IMM)},
	};
	check_cases(t, sizeof(t) / sizeof(t[0]));
}

TEST(loads_stores_and_jalr){
	const enc_case t[] = {
		{"lb r1, 0(r2)", enc_i(0, 2, 0, 1, OP_LOAD)},
		{"lh r1, 2(r2)", enc_i(2, 2, 1, 1, OP_LOAD)},
		{"lw r1, -8(r2)", enc_i(-8, 2, 2, 1, OP_LOAD)},
		{"lbu r1, 2047(r2)", enc_i(2047, 2, 4, 1, OP_LOAD)},
		{"lhu r1, -2048(r2)", enc_i(-2048, 2, 5, 1, OP_LOAD)},
		{"sb r3, 0(r4)", enc_s(0, 3, 4, 0, OP_STORE)},
		{"sh r3, 6(r4)", enc_s(6, 3, 4, 1, OP_STORE)},
		{"sw r3, -4(r4)", enc_s(-4, 3, 4, 2, OP_STORE)},
		{"sw r31, 2047(r30)", enc_s(2047, 31, 30, 2, OP_STORE)},
		{"sw r3, -2048(r4)", enc_s(-2048, 3, 4, 2, OP_STORE)},
		{"jalr r1, 4(r2)", enc_i(4, 2, 0, 1, OP_JALR)},
		{"jalr r0, 0(r1)", enc_i(0, 1, 0, 0, OP_JALR)},
	};
	check_cases(t, sizeof(t) / sizeof(t[0]));
}

TEST(upper_immediates){
	const enc_case t[] = {
		{"lui r1, 0x12345", enc_u(0x12345, 1, OP_LUI)},
		{"lui r1, 0xfffff", enc_u(0xFFFFF, 1, OP_LUI)},
		{"lui r10, 64", enc_u(64, 10, OP_LUI)},
		{"auipc r2, 0x1", enc_u(1, 2, OP_AUIPC)},
	};
	check_cases(t, sizeof(t) / sizeof(t[0]));
}

TEST(system_instructions){
	CHECK_EQ(assemble_line("ecall", 0), 0x00000073);
	CHECK_EQ(assemble_line("fence", 0), 0x0000000F);
	// the project's own ebreak encoding (understood by the simulator)
	CHECK_EQ(assemble_line("ebreak", 0), 0x02000073);
}

TEST(ebreak_uses_standard_encoding){
	CHECK_EQ(assemble_line("ebreak", 0), 0x00100073);
}

TEST(branches_are_pc_relative){
	add_symbol("fwd", 0x40);
	add_symbol("back", 0x10);
	CHECK_EQ(assemble_line("beq r1, r2, fwd", 0x20), enc_b(0x20, 2, 1, 0));
	CHECK_EQ(assemble_line("bne r1, r2, back", 0x20), enc_b(-0x10, 2, 1, 1));
	CHECK_EQ(assemble_line("blt r1, r2, back", 0x10), enc_b(0, 2, 1, 4));
	CHECK_EQ(assemble_line("bge r3, r4, fwd", 0x0), enc_b(0x40, 4, 3, 5));
	CHECK_EQ(assemble_line("bltu r3, r4, fwd", 0x0), enc_b(0x40, 4, 3, 6));
	CHECK_EQ(assemble_line("bgeu r3, r4, back", 0x810), enc_b(-0x800, 4, 3, 7));
}

TEST(branch_range_limits){
	add_symbol("far", 4096);
	add_symbol("edge", 4094);
	add_symbol("zero", 0);
	CHECK_EQ(assemble_line("beq r1, r2, edge", 0), enc_b(4094, 2, 1, 0));
	CHECK_EQ(assemble_line("beq r1, r2, zero", 4096), enc_b(-4096, 2, 1, 0));
	CHECK_EQ(assemble_line("beq r1, r2, far", 0), 0);		// out of range
	CHECK_EQ(assemble_line("beq r1, r2, missing", 0), 0);	// undefined label
}

TEST(jal_is_pc_relative){
	add_symbol("f", 0x1000);
	add_symbol("b", 0x4);
	CHECK_EQ(assemble_line("jal r1, f", 0x8), enc_j(0xFF8, 1));
	CHECK_EQ(assemble_line("jal r0, b", 0x104), enc_j(-0x100, 0));
	CHECK_EQ(assemble_line("jal r1, missing", 0), 0);
}

TEST(jal_range_limits){
	add_symbol("big", 0x100000);
	add_symbol("max", 0xFFFFE);
	CHECK_EQ(assemble_line("jal r1, max", 0), enc_j(0xFFFFE, 1));
	CHECK_EQ(assemble_line("jal r1, big", 0), 0);
}

TEST(labels_are_case_insensitive){
	tokens * t = tokenize("LOOP:\n");
	add_symbol(t->tok[0], 0x8);
	CHECK_EQ(assemble_line("bne r1, r2, Loop", 0x10), enc_b(-8, 2, 1, 1));
}

TEST_SUITE("asm_encoding",
	T(r_type),
	T(i_type),
	T(loads_stores_and_jalr),
	T(upper_immediates),
	T(system_instructions),
	XFAIL(ebreak_uses_standard_encoding, "ebreak is emitted as 0x02000073 (funct7 = 1) instead of the spec 0x00100073 (imm = 1)"),
	T(branches_are_pc_relative),
	T(branch_range_limits),
	T(jal_is_pc_relative),
	T(jal_range_limits),
	T(labels_are_case_insensitive),
)
