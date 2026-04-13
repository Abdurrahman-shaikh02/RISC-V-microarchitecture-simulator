#include "assembler.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
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

int read_dec(char * s){

	char * t = s;

	if(*t == '-') t++;

	while(*t){
		if(!isdigit(*t)) return INT_MIN;
		t++;
	}

	return atoi(s);

}


int read_hex(char * s){

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

	if(*t == '-') t++;

	if(*t == '0' && *(t+1) == 'x'){
		int imm = read_hex(s);
		if(imm >= -524288 && imm <= 524287) return imm;
	}else{
		int imm = read_dec(s);
		if(imm >= -524288 && imm <= 524287) return imm;
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


int main(){
	char * s = "assasd_()djh";
	s = read_l(s);
	if(s) printf("%s\n", s);

	int reg = read_s("0xa");
	printf("%d\n", reg);

}
