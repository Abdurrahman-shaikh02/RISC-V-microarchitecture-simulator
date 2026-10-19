// src/execution/control.c: instruction decoder / control signal generation
//
// These tests check the externally meaningful control signals of every RV32I
// instruction (format, register write back, memory access kind, jump/branch,
// system call). The ALU selection signals are checked indirectly, by actually
// executing instructions, in test_ex.c.
#include "sim_support.h"

static cu_output dec(uint32_t ir){
	sim_quiet();
	return decode(ir);
}

TEST(get_format_of_every_opcode){
	CHECK_EQ(get_format(OP_REG), 'r');
	CHECK_EQ(get_format(OP_IMM), 'i');
	CHECK_EQ(get_format(OP_LOAD), 'i');
	CHECK_EQ(get_format(OP_JALR), 'i');
	CHECK_EQ(get_format(OP_SYSTEM), 'i');
	CHECK_EQ(get_format(OP_FENCE), 'i');
	CHECK_EQ(get_format(OP_STORE), 's');
	CHECK_EQ(get_format(OP_BRANCH), 'b');
	CHECK_EQ(get_format(OP_LUI), 'u');
	CHECK_EQ(get_format(OP_AUIPC), 'u');
	CHECK_EQ(get_format(OP_JAL), 'j');
	CHECK_EQ(get_format(0x7F), 0);
	CHECK_EQ(get_format(0x0B), 0);
}

TEST(zero_word_decodes_as_bubble){
	cu_output o = dec(0);
	CHECK_EQ(o.format, 0);
	CHECK_EQ(o.cs_wb.wb, 0);
	CHECK_EQ(o.cs_ma.mem, 0);
	CHECK_EQ(o.cs_ma.jump_or_branch, 0);
	CHECK_EQ(o.cs_ex.SYSTEM_CALL, 0);
}

TEST(r_type_writes_back_without_memory){
	uint32_t f3[] = {0, 0, 1, 2, 3, 4, 5, 5, 6, 7};
	uint32_t f7[] = {0, 0x20, 0, 0, 0, 0, 0, 0x20, 0, 0};
	for(int i = 0; i < 10; i++){
		cu_output o = dec(enc_r(f7[i], 3, 2, f3[i], 1, OP_REG));
		CHECK_MSG(o.format == 'r' && o.cs_wb.wb == 1 && o.cs_ma.mem == 0 && o.cs_ma.jump_or_branch == 0,
			"r type funct3=%u funct7=%u: format=%c wb=%u mem=%u jb=%u", f3[i], f7[i], o.format, o.cs_wb.wb, o.cs_ma.mem, o.cs_ma.jump_or_branch);
		CHECK_EQ(o.cs_ex.source1, 1);	// rs1
		CHECK_EQ(o.cs_ex.source2, 0);	// rs2
	}
}

TEST(i_type_arithmetic_uses_immediate){
	uint32_t f3[] = {0, 2, 3, 4, 6, 7};
	for(int i = 0; i < 6; i++){
		cu_output o = dec(enc_i(-5, 2, f3[i], 1, OP_IMM));
		CHECK_EQ(o.format, 'i');
		CHECK_EQ(o.cs_wb.wb, 1);
		CHECK_EQ(o.cs_ma.mem, 0);
		CHECK_EQ(o.cs_ex.source1, 1);
		CHECK_EQ(o.cs_ex.source2, 1);	// immediate
	}
	// shifts
	CHECK_EQ(dec(enc_i(3, 2, 1, 1, OP_IMM)).cs_wb.wb, 1);		// slli
	CHECK_EQ(dec(enc_i(3, 2, 5, 1, OP_IMM)).cs_wb.wb, 1);		// srli
	CHECK_EQ(dec(enc_i(0x400 | 3, 2, 5, 1, OP_IMM)).cs_ex.sign, 1);	// srai
	CHECK_EQ(dec(enc_i(3, 2, 5, 1, OP_IMM)).cs_ex.sign, 0);		// srli
}

TEST(loads_have_their_access_codes){
	// read_write_opcode: bit 3 = write, low bits = b, h, w, bu, hu
	struct { uint32_t f3; uint32_t code; } t[] = {{0, 0b0000}, {1, 0b0001}, {2, 0b0010}, {4, 0b0011}, {5, 0b0100}};
	for(int i = 0; i < 5; i++){
		cu_output o = dec(enc_i(8, 2, t[i].f3, 1, OP_LOAD));
		CHECK_EQ(o.format, 'i');
		CHECK_EQ(o.cs_ma.mem, 1);
		CHECK_EQ(o.cs_ma.read_write_opcode, t[i].code);
		CHECK_EQ(o.cs_wb.wb, 1);
	}
}

TEST(stores_have_their_access_codes_and_no_writeback){
	struct { uint32_t f3; uint32_t code; } t[] = {{0, 0b1000}, {1, 0b1001}, {2, 0b1010}};
	for(int i = 0; i < 3; i++){
		cu_output o = dec(enc_s(8, 3, 2, t[i].f3, OP_STORE));
		CHECK_EQ(o.format, 's');
		CHECK_EQ(o.cs_ma.mem, 1);
		CHECK_EQ(o.cs_ma.read_write_opcode, t[i].code);
		CHECK_EQ(o.cs_wb.wb, 0);
	}
}

TEST(branches_are_marked_as_branch_without_writeback){
	uint32_t f3[] = {0, 1, 4, 5, 6, 7};
	for(int i = 0; i < 6; i++){
		cu_output o = dec(enc_b(8, 3, 2, f3[i]));
		CHECK_EQ(o.format, 'b');
		CHECK_EQ(o.cs_ma.jump_or_branch, 2);
		CHECK_EQ(o.cs_ex.type, 0b11);
		CHECK_EQ(o.cs_wb.wb, 0);
		CHECK_EQ(o.cs_ma.mem, 0);
	}
	// signed vs unsigned compare
	CHECK_EQ(dec(enc_b(8, 3, 2, 4)).cs_ex.sign, 1);	// blt
	CHECK_EQ(dec(enc_b(8, 3, 2, 5)).cs_ex.sign, 1);	// bge
	CHECK_EQ(dec(enc_b(8, 3, 2, 6)).cs_ex.sign, 0);	// bltu
	CHECK_EQ(dec(enc_b(8, 3, 2, 7)).cs_ex.sign, 0);	// bgeu
}

TEST(branch_prediction_bit_comes_from_predictor){
	BRANCH_PREDICTION_STRATEGY = NOT_TAKEN;
	CHECK_EQ(dec(enc_b(-8, 3, 2, 0)).cs_ma.branch_taken, 0);
	BRANCH_PREDICTION_STRATEGY = TAKEN;
	CHECK_EQ(dec(enc_b(8, 3, 2, 0)).cs_ma.branch_taken, 1);
	BRANCH_PREDICTION_STRATEGY = OFFSET;
	CHECK_EQ(dec(enc_b(-8, 3, 2, 0)).cs_ma.branch_taken, 1);
	CHECK_EQ(dec(enc_b(8, 3, 2, 0)).cs_ma.branch_taken, 0);
}

TEST(jumps_are_marked_as_jump_with_writeback){
	cu_output jal = dec(enc_j(16, 1));
	CHECK_EQ(jal.format, 'j');
	CHECK_EQ(jal.cs_ma.jump_or_branch, 1);
	CHECK_EQ(jal.cs_wb.wb, 1);
	CHECK_EQ(jal.cs_ex.branch_opcode, 0b100);

	cu_output jalr = dec(enc_i(4, 2, 0, 1, OP_JALR));
	CHECK_EQ(jalr.format, 'i');
	CHECK_EQ(jalr.cs_ma.jump_or_branch, 1);
	CHECK_EQ(jalr.cs_wb.wb, 1);
	CHECK_EQ(jalr.cs_ex.branch_opcode, 0b100);
	CHECK_EQ(jalr.cs_ex.source1, 1);	// target = rs1 + imm
}

TEST(upper_immediates_write_back){
	cu_output lui = dec(enc_u(0x12345, 1, OP_LUI));
	CHECK_EQ(lui.format, 'u');
	CHECK_EQ(lui.cs_wb.wb, 1);
	cu_output auipc = dec(enc_u(0x12345, 1, OP_AUIPC));
	CHECK_EQ(auipc.format, 'u');
	CHECK_EQ(auipc.cs_wb.wb, 1);
	CHECK_EQ(auipc.cs_ex.source1, 0);	// pc
}

TEST(system_instructions){
	cu_output ecall = dec(ENC_ECALL);
	CHECK_EQ(ecall.cs_ex.SYSTEM_CALL, 0b10);
	CHECK_EQ(ecall.cs_wb.wb, 0);

	cu_output ebreak = dec(ENC_EBREAK_PROJECT);
	CHECK_EQ(ebreak.cs_ex.SYSTEM_CALL, 1);
	CHECK_EQ(ebreak.cs_wb.wb, 0);

	cu_output fence = dec(0x0000000F);
	CHECK_EQ(fence.cs_ex.SYSTEM_CALL, 0);
	CHECK_EQ(fence.cs_wb.wb, 0);
	CHECK_EQ(fence.cs_ma.mem, 0);
}

TEST(standard_ebreak_encoding_is_a_breakpoint){
	// The spec encodes ebreak as imm=1 (0x00100073), which is what gcc emits.
	// The decoder looks at funct7 instead, so this decodes as an ecall.
	CHECK_EQ(dec(ENC_EBREAK_STANDARD).cs_ex.SYSTEM_CALL, 1);
}

TEST(gcc_fence_encoding_is_accepted){
	// gcc emits "fence iorw,iorw" = 0x0ff0000f
	cu_output o = dec(0x0FF0000F);
	CHECK_EQ(o.cs_wb.wb, 0);
	CHECK_EQ(o.cs_ma.mem, 0);
}

TEST(invalid_encodings_are_fatal){
	CHECK_FATAL(dec(0x0000007F), "invalid opcode, couldnt find a format");
	CHECK_FATAL(dec(enc_r(0x01, 3, 2, 0, 1, OP_REG)), "error in funct7 in r type");		// mul (M extension)
	CHECK_FATAL(dec(enc_r(0x01, 3, 2, 5, 1, OP_REG)), "error in funct7 in r type");		// divu
	CHECK_FATAL(dec(enc_i(0, 2, 3, 1, OP_LOAD)), "error in funct3");			// ld (RV64)
	CHECK_FATAL(dec(enc_s(0, 3, 2, 3, OP_STORE)), "error in funct3 in s type.");		// sd (RV64)
	CHECK_FATAL(dec(enc_b(8, 3, 2, 2)), "error in funct3 in b type.");
	CHECK_FATAL(dec(enc_i(0x7F, 2, 1, 1, OP_IMM) | (0x20u << 25)), "error in funct 7 in i type");	// slli with funct7 != 0
}

TEST_SUITE("control",
	T(get_format_of_every_opcode),
	T(zero_word_decodes_as_bubble),
	T(r_type_writes_back_without_memory),
	T(i_type_arithmetic_uses_immediate),
	T(loads_have_their_access_codes),
	T(stores_have_their_access_codes_and_no_writeback),
	T(branches_are_marked_as_branch_without_writeback),
	T(branch_prediction_bit_comes_from_predictor),
	T(jumps_are_marked_as_jump_with_writeback),
	T(upper_immediates_write_back),
	T(system_instructions),
	XFAIL(standard_ebreak_encoding_is_a_breakpoint, "decoder tells ecall/ebreak apart by funct7 instead of imm, the spec ebreak 0x00100073 is executed as ecall"),
	T(gcc_fence_encoding_is_accepted),
	T(invalid_encodings_are_fatal),
)
