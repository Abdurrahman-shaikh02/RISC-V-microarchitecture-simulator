#include "./../../include/assembler/assembler.h"

int get_number_of_operands(char * s){
	//table look up --- linear search
	for(int i = 0; i < i_table_length; i++){
		if(!strcmp(s, i_table[i].operation)){
			return i_table[i].number_of_operands;
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




