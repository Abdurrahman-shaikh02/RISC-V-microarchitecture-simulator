// src/assembler/core/{syntax_analyzer,semantics_analyzer,symbol_table}.c
#include "asm_support.h"

//------------------------------------------------------------------------------
// instruction table / syntax
//------------------------------------------------------------------------------

TEST(every_rv32i_mnemonic_is_known){
	const char * ops[] = {"add", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and",
		"addi", "slti", "sltiu", "xori", "ori", "andi", "slli", "srli", "srai", "lui", "auipc",
		"jal", "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu", "lb", "lh", "lw", "lbu", "lhu",
		"sb", "sh", "sw", "ecall", "ebreak", "fence", "nop"};
	for(unsigned i = 0; i < sizeof(ops) / sizeof(ops[0]); i++){
		char buf[16];
		snprintf(buf, sizeof(buf), "%s", ops[i]);
		const i_block * b = get_i_block(buf);
		CHECK_MSG(b != NULL && strcmp(b->operation, ops[i]) == 0, "mnemonic %s not found", ops[i]);
	}
	CHECK_EQ(sizeof(ops) / sizeof(ops[0]), i_table_length);
}

TEST(unknown_mnemonics){
	CHECK(get_i_block("mul") == NULL);
	CHECK(get_i_block("li") == NULL);
	CHECK(get_i_block("ADD") == NULL);	// the parser lowercases before lookup
}

TEST(syntax_operand_counts){
	CHECK_EQ(check_syntax(tokenize("add r1, r2, r3\n")->tok, 4, 0), 1);
	CHECK_EQ(check_syntax(tokenize("add r1, r2\n")->tok, 3, 0), 0);
	CHECK_EQ(check_syntax(tokenize("lw r1, 0(r2)\n")->tok, 3, 0), 1);
	CHECK_EQ(check_syntax(tokenize("lui r1, 5\n")->tok, 3, 0), 1);
	CHECK_EQ(check_syntax(tokenize("jal r1, x\n")->tok, 3, 0), 1);
	CHECK_EQ(check_syntax(tokenize("jalr r1, 0(r2)\n")->tok, 3, 0), 1);
	CHECK_EQ(check_syntax(tokenize("ecall\n")->tok, 1, 0), 1);
	CHECK_EQ(check_syntax(tokenize("ecall r1\n")->tok, 2, 0), 0);
	CHECK_EQ(check_syntax(tokenize("mul r1, r2, r3\n")->tok, 4, 0), 0);
}

TEST(syntax_error_messages){
	char out[1024];
	int st;
	RUN_ISOLATED(out, st, { tokens * t = tokenize("mul r1, r2, r3\n"); check_syntax(t->tok, 4, 1); });
	CHECK_STR_EQ(out, "Invalid operation name\n");
	RUN_ISOLATED(out, st, { tokens * t = tokenize("add r1\n"); check_syntax(t->tok, 2, 1); });
	CHECK_STR_EQ(out, "Invalid number of operands\n");
}

//------------------------------------------------------------------------------
// operand readers
//------------------------------------------------------------------------------

TEST(registers){
	CHECK_EQ(read_r("r0"), 0);
	CHECK_EQ(read_r("r31"), 31);
	CHECK_EQ(read_r("r07"), 7);
	CHECK_EQ(read_r("r0x1f"), 31);
	CHECK_EQ(read_r("r32"), INT_MIN);
	CHECK_EQ(read_r("r-1"), INT_MIN);
	CHECK_EQ(read_r("x1"), INT_MIN);	// ABI/x names are not supported
	CHECK_EQ(read_r("sp"), INT_MIN);
	CHECK_EQ(read_r("r"), INT_MIN);
	CHECK_EQ(read_r("r1a"), INT_MIN);
}

TEST(i_immediates_are_12_bit_signed){
	CHECK_EQ(read_i("0"), 0);
	CHECK_EQ(read_i("2047"), 2047);
	CHECK_EQ(read_i("-2048"), -2048);
	CHECK_EQ(read_i("2048"), INT_MIN);
	CHECK_EQ(read_i("-2049"), INT_MIN);
	CHECK_EQ(read_i("0x7ff"), 2047);
	CHECK_EQ(read_i("-0x800"), -2048);
	CHECK_EQ(read_i("0x800"), INT_MIN);
	CHECK_EQ(read_i("0xfff"), INT_MIN);
	CHECK_EQ(read_i("12a"), INT_MIN);
	CHECK_EQ(read_i("0xg"), INT_MIN);
	CHECK_EQ(read_i(""), INT_MIN);
}

TEST(shift_amounts_are_5_bit){
	CHECK_EQ(read_s("0"), 0);
	CHECK_EQ(read_s("31"), 31);
	CHECK_EQ(read_s("0x1f"), 31);
	CHECK_EQ(read_s("32"), INT_MIN);
	CHECK_EQ(read_s("-1"), INT_MIN);
}

TEST(u_immediates_are_20_bit_unsigned){
	CHECK_EQ(read_u("0"), 0);
	CHECK_EQ(read_u("1048575"), 1048575);
	CHECK_EQ(read_u("0xfffff"), 0xFFFFF);
	CHECK_EQ(read_u("1048576"), INT_MIN);
	CHECK_EQ(read_u("0x100000"), INT_MIN);
	CHECK_EQ(read_u("-1"), INT_MIN);
	CHECK_EQ(read_u("-0x1"), INT_MIN);
}

TEST(labels){
	CHECK(read_l("loop") != NULL);
	CHECK(read_l("_start") != NULL);
	CHECK(read_l("l2_x9") != NULL);
	CHECK(read_l("1loop") == NULL);
	CHECK(read_l("lo-op") == NULL);
	CHECK(read_l("lo(op") == NULL);
}

TEST(memory_operands){
	int imm = 0;
	CHECK_EQ(read_a("8(r2)", &imm), 2);
	CHECK_EQ(imm, 8);
	CHECK_EQ(read_a("-2048(r31)", &imm), 31);
	CHECK_EQ(imm, -2048);
	CHECK_EQ(read_a("0x10(r0)", &imm), 0);
	CHECK_EQ(imm, 16);
	CHECK_EQ(read_a("0(r1)", &imm), 1);
	CHECK_EQ(read_a("(r1)", &imm), INT_MIN);	// the offset is mandatory
	CHECK_EQ(read_a("8(r32)", &imm), INT_MIN);
	CHECK_EQ(read_a("2048(r1)", &imm), INT_MIN);
	CHECK_EQ(read_a("8r2", &imm), INT_MIN);
	CHECK_EQ(read_a("8(r2", &imm), INT_MIN);
}

TEST(semantics_per_operand_type){
	struct { const char * line; int ok; } t[] = {
		{"add r1, r2, r3\n", 1}, {"add r1, r2, 3\n", 0}, {"add r1, r2, r32\n", 0},
		{"addi r1, r2, -5\n", 1}, {"addi r1, r2, r3\n", 0}, {"addi r1, r2, 4096\n", 0},
		{"slli r1, r2, 31\n", 1}, {"slli r1, r2, 32\n", 0},
		{"lui r1, 0xfffff\n", 1}, {"lui r1, -1\n", 0},
		{"jal r1, loop\n", 1}, {"jal r1, 12\n", 0},
		{"jalr r1, 4(r2)\n", 1}, {"jalr r1, r2\n", 0},
		{"beq r1, r2, done\n", 1}, {"beq r1, 5, done\n", 0},
		{"lw r1, 8(r2)\n", 1}, {"lw r1, r2\n", 0}, {"sw r1, -4(r2)\n", 1},
		{"ecall\n", 1}, {"nop\n", 1},
	};
	for(unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++){
		tokens * tk = tokenize(t[i].line);
		int got = check_semantics(tk->tok, 0);
		CHECK_MSG(got == t[i].ok, "check_semantics(\"%.*s\") = %d, expected %d", (int)strlen(t[i].line) - 1, t[i].line, got, t[i].ok);
	}
}

TEST(semantics_error_messages){
	char out[1024];
	int st;
	RUN_ISOLATED(out, st, { tokens * t = tokenize("add r1, r2, 3\n"); check_semantics(t->tok, 1); });
	CHECK_STR_EQ(out, "Invalid operand 3\n");
	RUN_ISOLATED(out, st, { tokens * t = tokenize("add x1, r2, r3\n"); check_semantics(t->tok, 1); });
	CHECK_STR_EQ(out, "Invalid operand 1\n");
}

//------------------------------------------------------------------------------
// symbol table
//------------------------------------------------------------------------------

TEST(symbols_add_and_lookup){
	CHECK_EQ(add_symbol("loop", 8), 1);
	CHECK_EQ(add_symbol("done", 40), 1);
	CHECK_EQ(get_address("loop"), 8);
	CHECK_EQ(get_address("done"), 40);
	CHECK_EQ(get_address("missing"), -1);
	CHECK(get_symbol("loop") != NULL);
	CHECK(get_symbol("lo") == NULL);
}

TEST(symbols_reject_duplicates_and_long_names){
	CHECK_EQ(add_symbol("loop", 8), 1);
	CHECK_EQ(add_symbol("loop", 12), -1);
	CHECK_EQ(get_address("loop"), 8);
	char longname[300];
	memset(longname, 'a', 299);
	longname[299] = 0;
	CHECK_EQ(add_symbol(longname, 0), -1);
}

TEST(symbols_free_table){
	add_symbol("a", 4);
	free_symbol_table();
	CHECK_EQ(get_address("a"), -1);
	CHECK_EQ(add_symbol("a", 8), 1);
	CHECK_EQ(get_address("a"), 8);
}

TEST_SUITE("asm_semantics",
	T(every_rv32i_mnemonic_is_known),
	T(unknown_mnemonics),
	T(syntax_operand_counts),
	T(syntax_error_messages),
	T(registers),
	T(i_immediates_are_12_bit_signed),
	T(shift_amounts_are_5_bit),
	T(u_immediates_are_20_bit_unsigned),
	T(labels),
	T(memory_operands),
	T(semantics_per_operand_type),
	T(semantics_error_messages),
	T(symbols_add_and_lookup),
	T(symbols_reject_duplicates_and_long_names),
	T(symbols_free_table),
)
