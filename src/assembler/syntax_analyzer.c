#include "./../../include/assembler/assembler.h"
#include <string.h>

typedef struct syntax_block {
	const char * operation;			//why not make a operation[32] --> cuz ur not going to edit it... its unnecessary to do so.
	int number_of_operands;
} syntax_block;

static const int table_length = 5;
static const syntax_block syntax_table[] = {
	{"add", 3},
	{"sub", 3},
	{"load", 1},
	{"store", 1},
	{"jmp", 1},
	//these for now

};

int get_number_of_operands(char * s){
	//table look up --- linear search
	for(int i = 0; i < table_length; i++){
		if(!strcmp(s, syntax_table[i].operation)){
			return syntax_table[i].number_of_operands;
		}
	}
	return -1;
}

int check_syntax(char ** array, int ntokens){
	//take operation name, and number of tokens read and 0 for error, 1 for OK
	
	if(!ntokens) return 1;
	int number_of_operands = get_number_of_operands(array[0]);

	if(number_of_operands == ntokens-1) return 1;
	return 0;
	
}




