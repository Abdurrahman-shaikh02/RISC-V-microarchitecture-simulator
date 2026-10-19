// src/execution/alu.c: arithmetic/logic units, comparators, immediate generator
#include "sim_support.h"

//------------------------------------------------------------------------------
// arithmetic_unit
//------------------------------------------------------------------------------

TEST(arithmetic_add){
	CHECK_EQ(arithmetic_unit(2, 3, 0), 5);
	CHECK_EQ(arithmetic_unit(0xFFFFFFFF, 1, 0), 0);		// wraps
	CHECK_EQ(arithmetic_unit(0x7FFFFFFF, 1, 0), 0x80000000);	// signed overflow wraps
	CHECK_EQ(arithmetic_unit(5, (uint32_t)-7, 0), (uint32_t)-2);
}

TEST(arithmetic_sub){
	CHECK_EQ(arithmetic_unit(5, 3, 1), 2);
	CHECK_EQ(arithmetic_unit(3, 5, 1), (uint32_t)-2);
	CHECK_EQ(arithmetic_unit(0, 1, 1), 0xFFFFFFFF);
	CHECK_EQ(arithmetic_unit(0x80000000, 1, 1), 0x7FFFFFFF);
}

TEST(arithmetic_invalid_opcode_is_fatal){
	CHECK_FATAL(arithmetic_unit(1, 2, 2), "invalid alu opcode");
}

//------------------------------------------------------------------------------
// logical_unit
//------------------------------------------------------------------------------

TEST(logical_and_or_xor){
	CHECK_EQ(logical_unit(0xF0F0F0F0, 0xFF00FF00, 0b000, 0), 0xF000F000);
	CHECK_EQ(logical_unit(0xF0F0F0F0, 0xFF00FF00, 0b001, 0), 0xFFF0FFF0);
	CHECK_EQ(logical_unit(0xF0F0F0F0, 0xFF00FF00, 0b010, 0), 0x0FF00FF0);
}

TEST(logical_shift_left_uses_low_5_bits){
	CHECK_EQ(logical_unit(1, 0, 0b011, 0), 1);
	CHECK_EQ(logical_unit(1, 31, 0b011, 0), 0x80000000);
	CHECK_EQ(logical_unit(1, 32, 0b011, 0), 1);		// shamt 32 -> 0
	CHECK_EQ(logical_unit(3, 0xFFFFFFE1, 0b011, 0), 6);	// only low 5 bits (=1)
}

TEST(logical_shift_right_logical_and_arithmetic){
	CHECK_EQ(logical_unit(0x80000000, 31, 0b100, 0), 1);
	CHECK_EQ(logical_unit(0x80000000, 31, 0b100, 1), 0xFFFFFFFF);
	CHECK_EQ(logical_unit(0x80000000, 4, 0b100, 1), 0xF8000000);
	CHECK_EQ(logical_unit(0x40000000, 4, 0b100, 1), 0x04000000);
	CHECK_EQ(logical_unit(0xF0, 36, 0b100, 0), 0x0F);	// shamt masked to 4
	// srai: the immediate carries 0x400 (funct7 bit) which must be masked off
	CHECK_EQ(logical_unit(0x80000000, 0x404, 0b100, 1), 0xF8000000);
}

TEST(logical_lui_and_auipc_paths){
	CHECK_EQ(logical_unit(0, 0x12345, 0b101, 0), 0x12345000);
	CHECK_EQ(logical_unit(0x100, 0x12345, 0b110, 0), 0x12345100);
	CHECK_EQ(logical_unit(0x100, 0xFFFFF, 0b110, 0), 0xFFFFF100);
}

TEST(logical_invalid_opcode_is_fatal){
	CHECK_FATAL(logical_unit(1, 2, 0b111, 0), "invalid logical unit opcode");
}

//------------------------------------------------------------------------------
// comparators
//------------------------------------------------------------------------------

TEST(less_than_signed_and_unsigned){
	CHECK_EQ(less_than_comparator((uint32_t)-1, 1, 1), 1);
	CHECK_EQ(less_than_comparator((uint32_t)-1, 1, 0), 0);
	CHECK_EQ(less_than_comparator(1, 1, 1), 0);
	CHECK_EQ(less_than_comparator(1, 1, 0), 0);
	CHECK_EQ(less_than_comparator(0, 1, 0), 1);
	CHECK_EQ(less_than_comparator(0x80000000, 0x7FFFFFFF, 1), 1);
	CHECK_EQ(less_than_comparator(0x80000000, 0x7FFFFFFF, 0), 0);
}

TEST(less_than_invalid_sign_is_fatal){
	CHECK_FATAL(less_than_comparator(1, 2, 2), "less than comparator failed.");
}

TEST(branch_comparator_eq_ne){
	CHECK_EQ(branch_comparator(7, 7, 0b000, 0), 1);
	CHECK_EQ(branch_comparator(7, 8, 0b000, 0), 0);
	CHECK_EQ(branch_comparator(7, 7, 0b001, 0), 0);
	CHECK_EQ(branch_comparator(7, 8, 0b001, 0), 1);
}

TEST(branch_comparator_lt_ge_signedness){
	uint32_t m1 = (uint32_t)-1;
	// blt / bge (signed)
	CHECK_EQ(branch_comparator(m1, 0, 0b010, 1), 1);
	CHECK_EQ(branch_comparator(0, m1, 0b010, 1), 0);
	CHECK_EQ(branch_comparator(m1, 0, 0b011, 1), 0);
	CHECK_EQ(branch_comparator(0, m1, 0b011, 1), 1);
	CHECK_EQ(branch_comparator(5, 5, 0b011, 1), 1);
	// bltu / bgeu (unsigned)
	CHECK_EQ(branch_comparator(m1, 0, 0b010, 0), 0);
	CHECK_EQ(branch_comparator(0, m1, 0b010, 0), 1);
	CHECK_EQ(branch_comparator(m1, 0, 0b011, 0), 1);
	CHECK_EQ(branch_comparator(5, 5, 0b011, 0), 1);
}

TEST(branch_comparator_invalid_opcode_is_fatal){
	CHECK_FATAL(branch_comparator(1, 2, 0b111, 0), "Invalid branch opcode.");
}

//------------------------------------------------------------------------------
// generate_immediate
//------------------------------------------------------------------------------

TEST(immediate_bubble_and_r_format_are_zero){
	sim_quiet();
	CHECK_EQ(generate_immediate(0, 0xFFFFFFFF), 0);
	CHECK_EQ(generate_immediate('r', 0xFFFFFFFF), 0);
}

TEST(immediate_i_format){
	sim_quiet();
	CHECK_EQ(generate_immediate('i', enc_i(0, 1, 0, 2, OP_IMM)), 0);
	CHECK_EQ(generate_immediate('i', enc_i(2047, 1, 0, 2, OP_IMM)), 2047);
	CHECK_EQ(generate_immediate('i', enc_i(-1, 1, 0, 2, OP_IMM)), (uint32_t)-1);
	CHECK_EQ(generate_immediate('i', enc_i(-2048, 1, 0, 2, OP_IMM)), (uint32_t)-2048);
	CHECK_EQ(generate_immediate('i', enc_i(-1792, 31, 7, 31, OP_LOAD)), (uint32_t)-1792);
}

TEST(immediate_s_format){
	sim_quiet();
	CHECK_EQ(generate_immediate('s', enc_s(0, 5, 6, 2, OP_STORE)), 0);
	CHECK_EQ(generate_immediate('s', enc_s(31, 5, 6, 2, OP_STORE)), 31);
	CHECK_EQ(generate_immediate('s', enc_s(32, 5, 6, 2, OP_STORE)), 32);
	CHECK_EQ(generate_immediate('s', enc_s(2047, 31, 31, 2, OP_STORE)), 2047);
	CHECK_EQ(generate_immediate('s', enc_s(-4, 5, 6, 2, OP_STORE)), (uint32_t)-4);
	CHECK_EQ(generate_immediate('s', enc_s(-2048, 5, 6, 2, OP_STORE)), (uint32_t)-2048);
}

TEST(immediate_b_format){
	sim_quiet();
	int32_t offsets[] = {0, 2, 4, 8, 0x7FE, 0x800, 0xFFE, -2, -4, -8, -0x800, -4096, 2046, -2048};
	for(unsigned i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++){
		uint32_t ir = enc_b(offsets[i], 3, 4, 0);
		CHECK_MSG(generate_immediate('b', ir) == (uint32_t)offsets[i],
			"b immediate %d decoded as %d", offsets[i], (int32_t)generate_immediate('b', ir));
	}
}

TEST(immediate_u_format_is_not_shifted){
	// the u immediate is returned as the raw 20 bit field, the ALU shifts it
	sim_quiet();
	CHECK_EQ(generate_immediate('u', enc_u(0x12345, 1, OP_LUI)), 0x12345);
	CHECK_EQ(generate_immediate('u', enc_u(0x00001, 1, OP_LUI)), 0x1);
	CHECK_EQ(generate_immediate('u', enc_u(0xFFFFF, 1, OP_LUI)) << 12, 0xFFFFF000);
}

TEST(immediate_j_format){
	sim_quiet();
	int32_t offsets[] = {0, 2, 4, 0x7FE, 0x800, 0xFFE, 0x1000, 0xFF000, 0xFFFFE, -2, -4, -0x800, -0x1000, -1048576};
	for(unsigned i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++){
		uint32_t ir = enc_j(offsets[i], 1);
		CHECK_MSG(generate_immediate('j', ir) == (uint32_t)offsets[i],
			"j immediate %d decoded as %d", offsets[i], (int32_t)generate_immediate('j', ir));
	}
}

TEST(immediate_invalid_format_is_fatal){
	sim_quiet();
	CHECK_FATAL(generate_immediate('x', 0), "invalid format for immediate generation");
}

TEST_SUITE("alu",
	T(arithmetic_add),
	T(arithmetic_sub),
	T(arithmetic_invalid_opcode_is_fatal),
	T(logical_and_or_xor),
	T(logical_shift_left_uses_low_5_bits),
	T(logical_shift_right_logical_and_arithmetic),
	T(logical_lui_and_auipc_paths),
	T(logical_invalid_opcode_is_fatal),
	T(less_than_signed_and_unsigned),
	T(less_than_invalid_sign_is_fatal),
	T(branch_comparator_eq_ne),
	T(branch_comparator_lt_ge_signedness),
	T(branch_comparator_invalid_opcode_is_fatal),
	T(immediate_bubble_and_r_format_are_zero),
	T(immediate_i_format),
	T(immediate_s_format),
	T(immediate_b_format),
	T(immediate_u_format_is_not_shifted),
	T(immediate_j_format),
	T(immediate_invalid_format_is_fatal),
)
