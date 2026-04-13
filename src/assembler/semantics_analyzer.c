#include "./../../include/assembler/assembler.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

// all of the following return register numbers except for read label which the label if its valid, and NULL if its an error
// support -ves for i and u
// support hex for all
//
//
int read_r(char * s);
int read_a(char * s, int * imm);
int read_i(char * s);
char * read_l(char * s);
int read_s(char * s);
int read_u(char * s);	//20 bit immediate

// lets get error output as - INT_MIN since immediates are a max of 20 bits...
int read_hex(char * s);
int read_dec(char * s);

int read_r(char * s){
	if(*s == 'r'){
		int valid = 1;
		char * t = ++s;

		while(*t){
			if(!isdigit(*t)) return INT_MIN;
			t++;
		}

		int reg = atoi(s);
		if(reg < 32 && reg >= 0) return reg;

		return INT_MIN;
	}
	return INT_MIN;
}


int read_i(char * s){
	char * t = s;
	if(*t == '-') t++;
	while(*t){
		if(!isdigit(*t)) return INT_MIN;
		t++;
	}
	int imm = atoi(s);

	if(imm >= -2048 && imm <= 2047) return imm;
	return INT_MIN;
}


int read_s(char * s){
	char * t = s;
	while(*t){
		if(!isdigit(*t)) return INT_MIN;
		t++;
	}
	int imm = atoi(s);

	if(imm >= 0 && imm <= 31) return imm;
	return INT_MIN;
}


int read_u(char * s){
	char * t = s;
	if(*t == '-') t++;
	while(*t){
		if(!isdigit(*t)) return INT_MIN;
		t++;
	}
	int imm = atoi(s);

	if(imm >= -524288 && imm <= 524287) return imm;
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


int main(){
	char * s = "assasd_()djh";
	s = read_l(s);
	if(s) printf("%s\n", s);

}
