#include "assembler.h"

// seperate function for each format... generate_i()... r, i, u, j, b, s
// switch case statements... the reparsing will be handled by the main function...

//remember theres going to be no errors here so no checking required now.... this is the second compiler pass.
//the only error that we can have now if when the label isnt found, or the offset was out of bounds...

int read_operand(char type, char * s){
	//reads single int operands only not a and l
	/*
	int reg = INT_MIN, imm = INT_MIN;
	char * l = s;
	*/
	switch(type){
		case 'r':
			return read_r(s);
		case 'i':
			return read_i(s);
		case 's':
			return read_s(s);
		case 'u':
			return read_u(s);
		default:
			return 0;

	}
	
}


int read_b_format(int opcode, char ** array, const i_block * block, int current_instruction_address){
	int arg1 = read_operand(block->op1, array[1]);
	opcode = opcode | (arg1 << 15);
	
	int arg2 = read_operand(block->op2, array[2]);
	opcode = opcode | (arg2 << 20);

	int label = get_address(array[3]);
	if(label == -1) return 0;	//return 0 if the label is not found
	
	int arg3 = label - current_instruction_address;
	if(!(arg3 >= -4096 && arg3 <= 4095)){	// its a 13 bit value ---> the lsb is always zero...
		return 0;
	}

	opcode = opcode | ((arg3 & 0x1E) << 7);
	opcode = opcode | ((arg3 & 0x7E0) << 20);
	opcode = opcode | ((arg3 & 0x800) >> 4);
	opcode = opcode | ((arg3 & 0x1000) << 19);

	return opcode;
}


int read_j_format(int opcode, char ** array, const i_block * block, int current_instruction_address){
	int arg1 = read_operand(block->op1, array[1]);
	opcode = opcode | (arg1 << 7);

	int label = get_address(array[2]);
	if(label == -1) return 0;	//return 0 if the label is not found
	
	int arg2 = label - current_instruction_address;
	if(!(arg2 >= -1048576 && arg2 <= 1048575)){	// its a 21 bit value ---> the lsb is always zero...
		return 0;
	}

	opcode = opcode | ((arg2 & 0x7FE) << 20);
	opcode = opcode | ((arg2 & 0x800) << 9);
	opcode = opcode | ((arg2 & 0xFF000));
	opcode = opcode | ((arg2 & 0x100000) << 11);

	return opcode;
}


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

	int arg1 = read_operand(block->op1, array[1]);
	opcode = opcode | (arg1 << 7);

	int arg2 = read_operand(block->op2, array[2]);
	opcode = opcode | (arg2 << 15);

	int arg3 = read_operand(block->op3, array[3]);
	opcode = opcode | (arg3 << 20);

	return opcode;
}


int get_opcode(char ** array, int current_instruction_address){
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
			return read_j_format(opcode, array, block, current_instruction_address);
		case 'b':
			return read_b_format(opcode, array, block, current_instruction_address);
		case 's':
			return read_s_format(opcode, array, block); 
	}
	return 0; // an error zero will never come from here... will always be found, if there was an error the semantics would catch it...
}


