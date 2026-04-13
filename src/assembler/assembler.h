#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>


#define MAX_TOKEN_CHAR 32
#define MAX_TOKENS_IN_A_LINE 4



int parse_label(char * s, char ** ret_array, int max_tokens, int max_chars); //returns 0 for an error, 1 for a success
int parse_operation(char * s, char ** ret_array, int max_tokens, int max_chars);	// returns 0 for a sytax error, 1 for a success
char * remove_comments(char * s);	// returns NULL if theres an error , user must check for error
int get_tokens (char * s, char ** ret_array, int max_tokens, int max_chars);	//max tokens in a line , max chars in each token; returns -1 for error, 0 for an operation, 1 for a label





// for the syntax analyzer
typedef struct i_block {
	const char * operation;			//why not make a operation[32] --> cuz ur not going to edit it... its unnecessary to do so.
	int number_of_operands;
	char op1;
	char op2;
	char op3;
	char format;
	int opcode;
	int funct3;
	int funct7;
} i_block;


const i_block * get_i_block(char * s);	//table look up --- linear search, returns NULL if not found
int check_syntax(char ** array, int ntokens);	//take operation name, and number of tokens read and 0 for error, 1 for OK


static const int i_table_length = 38;
static const i_block i_table[] = {
	// R-TYPE
	{"add", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b000, 0b0000000},
	{"sub", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b000, 0b0100000},
	{"sll", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b001, 0b0000000},
	{"slt", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b010, 0b0000000},
	{"sltu",3, 'r', 'r', 'r', 'r', 0b0110011, 0b011, 0b0000000},
	{"xor", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b100, 0b0000000},
	{"srl", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b101, 0b0000000},
	{"sra", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b101, 0b0100000},
	{"or",  3, 'r', 'r', 'r', 'r', 0b0110011, 0b110, 0b0000000},
	{"and", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b111, 0b0000000},

	// I-TYPE (ARITH)
	{"addi", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b000, 0},
	{"slti", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b010, 0},
	{"sltiu",3, 'r', 'r', 'i', 'i', 0b0010011, 0b011, 0},
	{"xori", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b100, 0},
	{"ori",  3, 'r', 'r', 'i', 'i', 0b0010011, 0b110, 0},
	{"andi", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b111, 0},

	// SHIFT IMM (SPECIAL I)
	{"slli", 3, 'r', 'r', 's', 'i', 0b0010011, 0b001, 0b0000000},	// s : 5 bit immediate
	{"srli", 3, 'r', 'r', 's', 'i', 0b0010011, 0b101, 0b0000000},
	{"srai", 3, 'r', 'r', 's', 'i', 0b0010011, 0b101, 0b0100000},

	// U-TYPE
	{"lui",   2, 'r', 'u', 0, 'u', 0b0110111, 0, 0},		// u : 20 bit immediate
	{"auipc", 2, 'r', 'u', 0, 'u', 0b0010111, 0, 0},

	// J-TYPE
	{"jal", 2, 'r', 'l', 0, 'j', 0b1101111, 0, 0},			// l : label

	// I-TYPE (JUMP REG)
	{"jalr", 3, 'r', 'r', 'i', 'i', 0b1100111, 0b000, 0},

	// B-TYPE
	{"beq",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b000, 0},
	{"bne",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b001, 0},
	{"blt",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b100, 0},
	{"bge",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b101, 0},
	{"bltu", 3, 'r', 'r', 'l', 'b', 0b1100011, 0b110, 0},
	{"bgeu", 3, 'r', 'r', 'l', 'b', 0b1100011, 0b111, 0},

	// LOADS (I-TYPE)
	{"lb",  2, 'r', 'a', 0, 'i', 0b0000011, 0b000, 0},		// a : indexed addressing mode
	{"lh",  2, 'r', 'a', 0, 'i', 0b0000011, 0b001, 0},
	{"lw",  2, 'r', 'a', 0, 'i', 0b0000011, 0b010, 0},
	{"lbu", 2, 'r', 'a', 0, 'i', 0b0000011, 0b100, 0},
	{"lhu", 2, 'r', 'a', 0, 'i', 0b0000011, 0b101, 0},

	// STORES (S-TYPE)
	{"sb", 2, 'r', 'a', 0, 's', 0b0100011, 0b000, 0},
	{"sh", 2, 'r', 'a', 0, 's', 0b0100011, 0b001, 0},
	{"sw", 2, 'r', 'a', 0, 's', 0b0100011, 0b010, 0},

	/*
	// SYSTEM
	{"ecall",  0, 0, 0, 0, 'i', 0b1110011, 0b000, 0},
	{"ebreak", 0, 0, 0, 0, 'i', 0b1110011, 0b000, 1},

	// FENCE
	{"fence", 0, 0, 0, 0, 'i', 0b0001111, 0b000, 0},
	*/

	// NOP (pseudo but useful)
	{"nop", 0, 0, 0, 0, 'i', 0b0110011, 0b000, 0}
};



