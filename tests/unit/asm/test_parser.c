// src/assembler/core/parser.c: tokenizer, comments, labels
#include "asm_support.h"

TEST(r_type_line){
	tokens * t = tokenize("add r1, r2, r3\n");
	CHECK_EQ(t->n, 4);
	CHECK_STR_EQ(t->tok[0], "add");
	CHECK_STR_EQ(t->tok[1], "r1");
	CHECK_STR_EQ(t->tok[2], "r2");
	CHECK_STR_EQ(t->tok[3], "r3");
}

TEST(whitespace_and_case_are_normalised){
	tokens * t = tokenize("\t  ADD   R1 ,R2,\tR3   \n");
	CHECK_EQ(t->n, 4);
	CHECK_STR_EQ(t->tok[0], "add");
	CHECK_STR_EQ(t->tok[1], "r1");
	CHECK_STR_EQ(t->tok[3], "r3");
}

TEST(operation_without_operands){
	CHECK_EQ(tokenize("ecall\n")->n, 1);
	CHECK_EQ(tokenize("  nop  \n")->n, 1);
}

TEST(memory_operand_is_one_token){
	tokens * t = tokenize("lw r1, -8(r2)\n");
	CHECK_EQ(t->n, 3);
	CHECK_STR_EQ(t->tok[2], "-8(r2)");
	t = tokenize("SW r1, 0X10(R2)\n");
	CHECK_STR_EQ(t->tok[2], "0x10(r2)");
}

TEST(label_definition){
	tokens * t = tokenize("Main_Loop:\n");
	CHECK_EQ(t->n, 0);
	CHECK_STR_EQ(t->tok[0], "main_loop");	// labels are case insensitive
	CHECK_EQ(tokenize("  loop:   \n")->n, 0);
}

TEST(label_must_be_alone_on_its_line){
	CHECK_EQ(tokenize("loop: addi r1, r1, 1\n")->n, -1);
}

TEST(label_operand_with_underscore){
	tokens * t = tokenize("jal r1, my_func\n");
	CHECK_EQ(t->n, 3);
	CHECK_STR_EQ(t->tok[2], "my_func");
}

TEST(comments){
	CHECK_EQ(tokenize("// a full line comment\n")->n, 10);
	CHECK_EQ(tokenize("    //indented comment\n")->n, 10);
	tokens * t = tokenize("addi r1, r0, 1   // trailing comment, with: (odd) chars\n");
	CHECK_EQ(t->n, 4);
	CHECK_STR_EQ(t->tok[3], "1");
	CHECK_EQ(tokenize("loop:   // label comment\n")->n, 0);
}

TEST(single_slash_is_an_error){
	CHECK_EQ(tokenize("addi r1, r0, 1 / 2\n")->n, -1);
}

TEST(syntax_errors){
	const char * bad[] = {
		"add r1, r2, r3, r4\n",		// too many tokens
		"add r1,, r2\n",			// empty operand
		"add r1, r2,\n",			// trailing comma
		"addi r1 r2, 3\n",			// missing comma
		"add$ r1\n",				// invalid character
		"add r1, r2, #3\n",
		"add, r1\n",				// comma right after the operation
		"add(r1)\n",
		"lw r1, 8(r2):\n",			// brackets and a colon
		"addi r1, r2, -1_\n",			// minus and underscore
		"my-label:\n",
	};
	for(unsigned i = 0; i < sizeof(bad) / sizeof(bad[0]); i++){
		tokens * t = tokenize(bad[i]);
		CHECK_MSG(t->n == -1, "\"%.*s\" tokenized to %d, expected -1", (int)strlen(bad[i]) - 1, bad[i], t->n);
	}
}

TEST(remove_comments_cuts_at_double_slash){
	char s[] = "add r1 // x\n";
	CHECK_STR_EQ(remove_comments(s), "add r1 \n");
	char ok[] = "add r1\n";
	CHECK_STR_EQ(remove_comments(ok), "add r1\n");
	char bad[] = "add / r1\n";
	CHECK(remove_comments(bad) == NULL);
}

TEST_SUITE("asm_parser",
	T(r_type_line),
	T(whitespace_and_case_are_normalised),
	T(operation_without_operands),
	T(memory_operand_is_one_token),
	T(label_definition),
	T(label_must_be_alone_on_its_line),
	T(label_operand_with_underscore),
	T(comments),
	T(single_slash_is_an_error),
	T(syntax_errors),
	T(remove_comments_cuts_at_double_slash),
)
