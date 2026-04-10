// #include "assembler.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#define MAX_TOKEN_CHAR 32
#define MAX_TOKENS_IN_A_LINE 4
// this parser takes in one line and returns an array of pointers to strings(tokens) not including commas, not including spaces nothing...
// its input is a char *
// also fixes case.
// only a max of 4 chars in one line, return NULL otherwise
// this simple parser will give error if not the following formats,
// <op>
// <op> <reg>
// <op> <reg>,<reg>
// <op> <reg>,<reg>,<reg>
//
//
// doesnt handle labels ... need to make it... thinking of making two functions 
// one for labels if a : is detected (allow underscore aswell, and no case changing)
// and another for operations if : (and _ ) not detected
// and a third to handle comments - using #

char ** get_tokens (char * s){
	//only for tokens max in each line
	char ** ret_array = (char **)malloc(sizeof(char *) * MAX_TOKENS_IN_A_LINE);

	//allocate memory for each token string
	for(int i = 0; i < MAX_TOKENS_IN_A_LINE; i++){
		ret_array[i] = (char *)malloc(sizeof(char) * MAX_TOKEN_CHAR);
	}

	//initialize all strings as 0
	for(int i = 0; i < MAX_TOKENS_IN_A_LINE; i++){
		char * l = ret_array[i];
		for(int j = 0; j < MAX_TOKEN_CHAR; j++){
			l[j] = 0;
		}
	}

	int i = 0; //keeps count in the ret_array
	
	char * t = s;
	//check for characters
	while(*t){
		if(!(isspace(*t) || isalnum(*t) || *t == ',')) return NULL;	//invalid character
		t++;
	}
	
	//remove spaces
	while(isspace(*s)){
		s++;
	}

	//fetch operation name
	t = ret_array[i];
	while(isalnum(*s)){
		*t = tolower(*s);
		s++;
		t++;
	}
	if(t == ret_array[i]) return NULL;	//no operation name
	*t = '\0';

	//if the operation name does not precedes a comma theres an error 
	if(*s == ','){
		return NULL;
	}

	//if theres no comma, there has to be a space or a null... null will be checked later... skip the spaces
	while(isspace(*s)){
		s++;
	}
	i++;


	while(*s){
		if(i>=MAX_TOKENS_IN_A_LINE){
			i = -1;
			break;
		}

		while(isspace(*s)){
			s++;
		}
		
		t = ret_array[i];
		while(isalnum(*s)){
			*t = tolower(*s);
			s++;
			t++;
		}
		if(t == ret_array[i]) return NULL;
		*t = '\0';

		while(isspace(*s)){
			s++;
		}

		if(*s == ','){
			i++;
			s++;
			if(!*s) return NULL;		//add s1,     --->     comma and the string ends the full while loop , hence the check
		}else if(isalnum(*s)){
			return NULL;
		}else{
			break;
		}
	}

	if(i == -1) return NULL;
	return ret_array;

}



int main(){
	char * s;

	char ** arr = get_tokens("ADD x1, x3, x2");

	if(!arr) return 0;

	for(int i = 0; i < 4; i++){
		printf("%s\n", arr[i]);

	}
	
}
