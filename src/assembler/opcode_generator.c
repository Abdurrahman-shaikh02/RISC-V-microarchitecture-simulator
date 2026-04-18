#include "assembler.h"

// seperate function for each format... generate_i()... r, i, u, j, b, s
// switch case statements... the reparsing will be handled by the main function...

//remember theres going to be no errors here so no checking required now.... this is the second compiler pass.

int read_operand(char type, char * s){
	//reads single int operands only not a and l
	
	int reg = INT_MIN, imm = INT_MIN;
	char * l = s;
	switch(type){
		case 'r':
			return read_r(s);
		case 'i':
			return read_i(s);
		case 's':
			return read_s(s);
		case 'u':
			return read_u(s);
	/*
		case 'l':
			l = read_l(s);
			if(l == NULL) return 0;
			return 1;
	
		case 'a':
			return read_a(s, &imm);
	*/
		default:
			return 0;

	}
	
}

// has bugs here test both with positive and negative offsets...
int read_s_format(int opcode, char ** array, const i_block * block){
	int arg1 = read_operand(block->op1, array[1]);
	opcode = opcode | (arg1 << 20);

	int arg2, arg3;
	if(block->op2 == 'a'){
		arg2 = read_a(array[2], &arg3);
	}else{
		arg2 = read_operand(block->op2, array[2]);
		arg3 = read_operand(block->op3, array[3]);
	}

	opcode = opcode | (arg2 << 15);

	opcode = opcode | ((arg3 & 0x1F) << 7);
	opcode = opcode | ((arg3 & 0xFE0) << 20);
	
	return opcode;
}


int read_u_format(int opcode, char ** array, const i_block * block){
	int arg1 = read_operand(block->op1, array[1]);
	opcode = opcode | (arg1 << 7);

	int arg2 = read_operand(block->op2, array[2]);
	opcode = opcode | (arg2 << 12);

	return opcode;
}


int read_i_format(int opcode, char ** array, const i_block * block){
	int arg1 = read_operand(block->op1, array[1]);
	opcode = opcode | (arg1 << 7);
	
	int arg2, arg3;
	if(block->op2 == 'a'){
		arg2 = read_a(array[2], &arg3);
	}else{
		arg2 = read_operand(block->op2, array[2]);
		arg3 = read_operand(block->op3, array[3]);
	}

	opcode = opcode | (arg2 << 15);
	opcode = opcode | (arg3 << 20);

	return opcode;
}


int read_r_format(int opcode, char ** array, const i_block * block){

	// handles the nop command here
	if(!strcmp(array[0], "nop")) return opcode;

	int arg1 = read_operand(block->op1, array[1]);
	opcode = opcode | (arg1 << 7);

	int arg2 = read_operand(block->op2, array[2]);
	opcode = opcode | (arg2 << 15);

	int arg3 = read_operand(block->op3, array[3]);
	opcode = opcode | (arg3 << 20);

	return opcode;
}


int get_opcode(char ** array, int address){
	int opcode = 0;
	const i_block * block = get_i_block(array[0]);

	int format = block->format;

	// since in our table, funt3 and funct 7 are zero if not required... and opcode is in the same position...
	opcode = opcode | block->opcode;
	opcode = opcode | (block->funct3 << 12);
	opcode = opcode | (block->funct7 << 25);


	switch(format){
		case 'r':
			return read_r_format(opcode, array, block);
		case 'i':
			return read_i_format(opcode, array, block);
		case 'u':
			return read_u_format(opcode, array, block);
		case 'j':
		case 'b':
		case 's':
			return read_s_format(opcode, array, block); //has bugsssss------------------------------------------
	}
	return -1;
}


