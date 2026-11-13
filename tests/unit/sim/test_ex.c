// src/pipeline/id.c + src/pipeline/ex.c: every RV32I instruction is decoded
// by the real ID stage and executed by the real EX stage; the result in ex_ma
// is checked against the ISA spec.
#include "sim_support.h"

#define RS1 2
#define RS2 3
#define RD 1

static uint32_t run_r(uint32_t funct7, uint32_t funct3, uint32_t a, uint32_t b){
	sim_setup(-1);
	reg_file[RS1] = a;
	reg_file[RS2] = b;
	return sim_execute(enc_r(funct7, RS2, RS1, funct3, RD, OP_REG), 0x100);
}

static uint32_t run_i(uint32_t funct3, uint32_t a, int32_t imm){
	sim_setup(-1);
	reg_file[RS1] = a;
	return sim_execute(enc_i(imm, RS1, funct3, RD, OP_IMM), 0x100);
}

typedef struct { const char * name; uint32_t f7, f3, a, b, expected; } r_case;
typedef struct { const char * name; uint32_t f3; uint32_t a; int32_t imm; uint32_t expected; } i_case;

TEST(r_type_semantics){
	const r_case t[] = {
		{"add", 0x00, 0, 5, 7, 12},
		{"add wraps", 0x00, 0, 0xFFFFFFFF, 2, 1},
		{"sub", 0x20, 0, 5, 7, (uint32_t)-2},
		{"sub min", 0x20, 0, 0x80000000, 1, 0x7FFFFFFF},
		{"sll", 0x00, 1, 1, 31, 0x80000000},
		{"sll masks shamt", 0x00, 1, 1, 33, 2},
		{"slt true", 0x00, 2, (uint32_t)-5, 3, 1},
		{"slt false", 0x00, 2, 3, (uint32_t)-5, 0},
		{"slt equal", 0x00, 2, 3, 3, 0},
		{"sltu", 0x00, 3, 3, (uint32_t)-5, 1},
		{"sltu false", 0x00, 3, (uint32_t)-5, 3, 0},
		{"xor", 0x00, 4, 0xFF00FF00, 0x0FF00FF0, 0xF0F0F0F0},
		{"srl", 0x00, 5, 0x80000000, 31, 1},
		{"srl masks shamt", 0x00, 5, 0x80, 0x24, 0x8},
		{"sra", 0x20, 5, 0x80000000, 31, 0xFFFFFFFF},
		{"sra positive", 0x20, 5, 0x40000000, 30, 1},
		{"or", 0x00, 6, 0xF0, 0x0F, 0xFF},
		{"and", 0x00, 7, 0xF0F0, 0xFF00, 0xF000},
	};
	for(unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++){
		uint32_t got = run_r(t[i].f7, t[i].f3, t[i].a, t[i].b);
		CHECK_MSG(got == t[i].expected, "%s: 0x%08x op 0x%08x = 0x%08x, expected 0x%08x",
			t[i].name, t[i].a, t[i].b, got, t[i].expected);
		CHECK_EQ(ex_ma.nrd, RD);
		CHECK_EQ(ex_ma.cs_wb.wb, 1);
	}
}

TEST(i_type_semantics){
	const i_case t[] = {
		{"addi", 0, 5, 7, 12},
		{"addi negative", 0, 5, -7, (uint32_t)-2},
		{"addi min imm", 0, 0, -2048, (uint32_t)-2048},
		{"addi max imm", 0, 0, 2047, 2047},
		{"slti true", 2, (uint32_t)-5, 3, 1},
		{"slti false", 2, 3, -5, 0},
		{"sltiu imm sign extended", 3, 5, -1, 1},	// 5 < 0xFFFFFFFF
		{"sltiu false", 3, 0xFFFFFFFF, -1, 0},
		{"sltiu seqz idiom", 3, 0, 1, 1},
		{"xori", 4, 0x0F, 0xFF, 0xF0},
		{"xori not idiom", 4, 0x12345678, -1, 0xEDCBA987},
		{"ori", 6, 0xF0, 0x0F, 0xFF},
		{"ori sign extended", 6, 0, -2048, 0xFFFFF800},
		{"andi", 7, 0xFFFF, 0x0F0, 0xF0},
		{"andi sign extended", 7, 0x12345678, -16, 0x12345670},
		{"slli", 1, 3, 4, 48},
		{"slli 31", 1, 1, 31, 0x80000000},
		{"srli", 5, 0x80000000, 31, 1},
		{"srai", 5, 0x80000000, 0x400 | 4, 0xF8000000},
	};
	for(unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++){
		uint32_t got = run_i(t[i].f3, t[i].a, t[i].imm);
		CHECK_MSG(got == t[i].expected, "%s: 0x%08x op %d = 0x%08x, expected 0x%08x",
			t[i].name, t[i].a, t[i].imm, got, t[i].expected);
	}
}

TEST(lui){
	sim_setup(-1);
	CHECK_EQ(sim_execute(enc_u(0x12345, RD, OP_LUI), 0x100), 0x12345000);
	CHECK_EQ(sim_execute(enc_u(0xFFFFF, RD, OP_LUI), 0x100), 0xFFFFF000);
	CHECK_EQ(sim_execute(enc_u(0, RD, OP_LUI), 0x100), 0);
}

TEST(auipc_adds_the_instruction_pc){
	sim_setup(-1);
	CHECK_EQ(sim_execute(enc_u(0x1, RD, OP_AUIPC), 0x100), 0x1100);
	CHECK_EQ(sim_execute(enc_u(0, RD, OP_AUIPC), 0x2468), 0x2468);
	CHECK_EQ(sim_execute(enc_u(0xFFFFF, RD, OP_AUIPC), 0x2000), 0x1000);
}

TEST(jal_links_and_targets){
	sim_setup(-1);
	sim_execute(enc_j(0x40, RD), 0x100);
	CHECK_EQ(ex_ma.result, 0x104);		// link value = pc + 4
	CHECK_EQ(ex_ma.PC_next, 0x140);		// target
	CHECK_EQ(ex_ma.cs_ma.jump_or_branch, 1);
	CHECK_EQ(ex_ma.cs_ma.branch_taken & 0b10, 0b10);

	sim_execute(enc_j(-0x100, RD), 0x200);
	CHECK_EQ(ex_ma.PC_next, 0x100);
}

TEST(jalr_links_and_targets){
	sim_setup(-1);
	reg_file[RS1] = 0x1000;
	sim_execute(enc_i(-4, RS1, 0, RD, OP_JALR), 0x100);
	CHECK_EQ(ex_ma.result, 0x104);
	CHECK_EQ(ex_ma.PC_next, 0xFFC);
	CHECK_EQ(ex_ma.cs_ma.jump_or_branch, 1);
}

TEST(jalr_clears_the_lowest_target_bit){
	// spec: target = (rs1 + imm) & ~1
	sim_setup(-1);
	reg_file[RS1] = 0x1001;
	sim_execute(enc_i(0, RS1, 0, RD, OP_JALR), 0x100);
	CHECK_EQ(ex_ma.PC_next, 0x1000);
}

typedef struct { const char * name; uint32_t f3, a, b, taken; } b_case;

TEST(branch_outcomes){
	const b_case t[] = {
		{"beq taken", 0, 5, 5, 1}, {"beq not", 0, 5, 6, 0},
		{"bne taken", 1, 5, 6, 1}, {"bne not", 1, 5, 5, 0},
		{"blt taken", 4, (uint32_t)-1, 0, 1}, {"blt not", 4, 0, (uint32_t)-1, 0}, {"blt equal", 4, 3, 3, 0},
		{"bge taken", 5, 0, (uint32_t)-1, 1}, {"bge equal", 5, 3, 3, 1}, {"bge not", 5, (uint32_t)-1, 0, 0},
		{"bltu taken", 6, 0, (uint32_t)-1, 1}, {"bltu not", 6, (uint32_t)-1, 0, 0}, {"bltu equal", 6, 3, 3, 0},
		{"bgeu taken", 7, (uint32_t)-1, 0, 1}, {"bgeu equal", 7, 3, 3, 1}, {"bgeu not", 7, 0, (uint32_t)-1, 0},
	};
	for(unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++){
		sim_setup(-1);
		reg_file[RS1] = t[i].a;
		reg_file[RS2] = t[i].b;
		sim_execute(enc_b(-0x20, RS2, RS1, t[i].f3), 0x100);
		uint32_t actual = (ex_ma.cs_ma.branch_taken >> 1) & 1;
		CHECK_MSG(actual == t[i].taken, "%s: actual taken bit %u, expected %u", t[i].name, actual, t[i].taken);
		CHECK_EQ(ex_ma.PC_next, 0xE0);	// branch target (pc + imm)
		CHECK_EQ(ex_ma.result, 0x104);	// fall through address
		CHECK_EQ(ex_ma.cs_ma.jump_or_branch, 2);
		CHECK_EQ(ex_ma.cs_wb.wb, 0);
	}
}

TEST(branch_keeps_prediction_bit){
	sim_setup(-1);
	BRANCH_PREDICTION_STRATEGY = TAKEN;
	reg_file[RS1] = 1;
	sim_execute(enc_b(16, RS2, RS1, 0), 0x100);	// beq 1, 0 -> not taken, predicted taken
	CHECK_EQ(ex_ma.cs_ma.branch_taken, 0b01);
}

TEST(load_and_store_compute_address){
	sim_setup(-1);
	reg_file[RS1] = 0x40000;
	sim_execute(enc_i(-8, RS1, 2, RD, OP_LOAD), 0x100);
	CHECK_EQ(ex_ma.result, 0x40000 - 8);
	CHECK_EQ(ex_ma.cs_ma.mem, 1);

	sim_setup(-1);
	reg_file[RS1] = 0x40000;
	reg_file[RS2] = 0xCAFE;
	sim_execute(enc_s(12, RS2, RS1, 2, OP_STORE), 0x100);
	CHECK_EQ(ex_ma.result, 0x4000C);
	CHECK_EQ(ex_ma.R2, 0xCAFE);		// store data travels in R2
	CHECK_EQ(ex_ma.nrs2, RS2);
}

TEST(store_data_is_forwarded_in_ex){
	sim_setup(-1);
	reg_file[RS1] = 0x40000;
	reg_file[RS2] = 0x1111;		// stale value
	sim_decode_into_id_ex(enc_s(0, RS2, RS1, 2, OP_STORE), 0x100);
	wb_if.nrd = RS2;		// the producer just wrote back
	wb_if.result = 0x2222;
	wb_if.cs_wb.wb = 1;
	ex_stage();
	CHECK_EQ(ex_ma.R2, 0x2222);
}

TEST(operands_are_forwarded_in_ex){
	sim_setup(-1);
	reg_file[RS1] = 1;
	reg_file[RS2] = 2;
	sim_decode_into_id_ex(enc_r(0, RS2, RS1, 0, RD, OP_REG), 0x100);
	ex_ma.nrd = RS1;
	ex_ma.result = 100;
	ex_ma.cs_wb.wb = 1;
	wb_if.nrd = RS2;
	wb_if.result = 20;
	wb_if.cs_wb.wb = 1;
	ex_stage();
	CHECK_EQ(ex_ma.result, 120);
}

TEST(ecall_and_ebreak_raise_flags_only){
	sim_setup(-1);
	sim_execute(ENC_ECALL, 0x100);
	CHECK_EQ(TRAP, 1);
	CHECK_EQ(PAUSE, 0);

	sim_setup(-1);
	sim_execute(ENC_EBREAK_PROJECT, 0x100);
	CHECK_EQ(PAUSE, 1);
	CHECK_EQ(TRAP, 0);
}

TEST(bubble_passes_through){
	sim_setup(-1);
	sim_execute(0, 0);
	CHECK_EQ(ex_ma.cs_wb.wb, 0);
	CHECK_EQ(ex_ma.cs_ma.mem, 0);
	CHECK_EQ(ex_ma.cs_ma.jump_or_branch, 0);
}

TEST_SUITE("execute",
	T(r_type_semantics),
	T(i_type_semantics),
	T(lui),
	T(auipc_adds_the_instruction_pc),
	T(jal_links_and_targets),
	T(jalr_links_and_targets),
	XFAIL(jalr_clears_the_lowest_target_bit, "jalr target is rs1+imm without clearing bit 0 (spec: & ~1)"),
	T(branch_outcomes),
	T(branch_keeps_prediction_bit),
	T(load_and_store_compute_address),
	T(store_data_is_forwarded_in_ex),
	T(operands_are_forwarded_in_ex),
	T(ecall_and_ebreak_raise_flags_only),
	T(bubble_passes_through),
)
