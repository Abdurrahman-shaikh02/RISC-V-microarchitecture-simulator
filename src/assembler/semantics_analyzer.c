#include "assembler.h"
#include <limits.h>

// all of the following return register numbers except for read label which the label if its valid, and NULL if its an error
// support -ves for i and u
// support hex for all
//
//
// lets get error output as - INT_MIN since immediates are a max of 20 bits...

int read_dec(char * s){
	if(!*s) return INT_MIN;

	char * t = s;

	if(*t == '-') t++;

	while(*t){
		if(!isdigit(*t)) return INT_MIN;
		t++;
	}

	return atoi(s);

}


int read_hex(char * s){
	if(!*s) return INT_MIN;

	char * t = s;
	if(*t == '-') t++;

	if(*t == '0' && *(t+1) == 'x'){
		t = t + 2;
	}

	while(*t){
		if(!(isdigit(*t) || *t == 'a' || *t == 'b' || *t == 'c' || *t == 'd' || *t == 'e' || *t == 'f')) return INT_MIN;
		t++;
	}

	char *end; //useless

	return (int) strtol(s, &end, 16);

}


int read_r(char * s){
	if(*s == 'r'){
		s++;
		char * t = s;
		if(*t == '-') t++;
		if(*t == '0' && *(t+1) == 'x'){
			int reg = read_hex(s);
			if(reg >= 0 && reg < 32) return reg;
		}else{
			int reg = read_dec(s);
			if(reg >= 0 && reg < 32) return reg;
		}
	}

	return INT_MIN;
}


int read_i(char * s){

	char * t = s;

	if(*t == '-') t++;

	if(*t == '0' && *(t+1) == 'x'){
		int imm = read_hex(s);
		if(imm >= -2048 && imm <= 2047) return imm;
	}else{
		int imm = read_dec(s);
		if(imm >= -2048 && imm <= 2047) return imm;
	}

	return INT_MIN;
}


int read_s(char * s){
	char * t = s;

	if(*t == '-') t++;

	if(*t == '0' && *(t+1) == 'x'){
		int imm = read_hex(s);
		if(imm >= 0 && imm <= 31) return imm;
	}else{
		int imm = read_dec(s);
		if(imm >= 0 && imm <= 31) return imm;
	}

	return INT_MIN;
}


int read_u(char * s){
	char * t = s;
	int negative = 0;
	if(*t == '-'){
		negative = 1;
		t++;
	}

	if(*t == '0' && *(t+1) == 'x'){
		if(negative) return INT_MIN;
		int imm = read_hex(s);
		if(imm >= 0 && imm <= 1048575) return imm;
	}else{
		int imm = read_dec(s);
		if(imm >= 0 && imm <= 1048575) return imm;
	}

	return INT_MIN;
}


char * read_l(char * s){
	char * t = s;
	if(isdigit(*t)) return NULL;
	while(*t){
		if(!(isalnum(*t) || *t == '_')) return NULL;
		t++;
	}

	return s;
}


int read_a(char * s, int * imm){
	// if ANY error return INT_MIN
	char * t = s;
	char offset[32];
	int i = 0;
	char reg[8];
	int j = 0;

	while(*t != '('){
		if(*t == 0) return INT_MIN;
		if(i >= 32) return INT_MIN;
		offset[i] = *t;
		t++;
		i++;
	}
	if(i >= 32) return INT_MIN;
	offset[i] = 0;
	t++;

	while(*t != ')'){
		if(*t == 0) return INT_MIN;
		if(j >= 8) return INT_MIN;
		reg[j] = *t;
		t++;
		j++;
	}
	if(j >= 8) return INT_MIN;
	reg[j] = 0;


	*imm = read_i(offset);
	if(*imm == INT_MIN) return INT_MIN;

	return read_r(reg);
}


int check_operand(char type, char * s){
	// 0 for illegal arg, 1 for legal
	
	int reg = INT_MIN, imm = INT_MIN;
	char * l = s;
	switch(type){
		case 'r':
			reg = read_r(s);
			if(reg == INT_MIN) return 0;
			return 1;
		case 'i':
			imm = read_i(s);
			if(imm == INT_MIN) return 0;
			return 1;
		case 's':
			imm = read_s(s);
			if(imm == INT_MIN) return 0;
			return 1;
		case 'u':
			imm = read_u(s);
			if(imm == INT_MIN) return 0;
			return 1;
		case 'l':
			l = read_l(s);
			if(l == NULL) return 0;
			return 1;
		case 'a':
			reg = read_a(s, &imm);
			if(reg == INT_MIN) return 0;
			return 1;
		default:
			return 0;

	}
	
}


int check_semantics(char ** instruction, int errors){
	// 0 for illegal instruction, 1 for legal instruction; error flag set --> print errors...
	i_block * block = NULL;
	block = (i_block *)get_i_block(instruction[0]);

	if(block == NULL){
		if(errors) printf("Invalid operation name\n");
		return 0;
	}

	int noperands = block->number_of_operands;

	if(noperands > 0) if(!check_operand(block->op1, instruction[1])){
		if(errors) printf("Invalid operand 1\n");
		return 0;
	}

	if(noperands > 1) if(!check_operand(block->op2, instruction[2])){
		if(errors) printf("Invalid operand 2\n");
		return 0;
	}
	if(noperands > 2) if(!check_operand(block->op3, instruction[3])){
		if(errors) printf("Invalid operand 3\n");
		return 0;
	}

	return 1;

}

/*
int main(){
	
	int imm;
	int reg = read_a("-45(r2)", &imm);
	printf("%d + %d\n", imm, reg);
	

	// printf("%d\n", check_operand('r', "-45(r2)"));
	
	int reg = read_r("x1");
	printf("%d\n", reg);

}
*/
