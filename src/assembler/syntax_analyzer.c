#include "./../../include/assembler/assembler.h"

 const i_block * get_i_block(char * s){
	// returns a const i_block * to the block found , NULL if not found
	//table look up --- linear search
	for(int i = 0; i < i_table_length; i++){
		if(!strcmp(s, i_table[i].operation)){
			return (const i_block *)(i_table + i);
		}
	}
	return NULL;
}

int check_syntax(char ** array, int ntokens){
	//take operation name, and number of tokens read and 0 for error, 1 for OK
	
	if(!ntokens) return 1;
	const i_block * block = get_i_block(array[0]);

	if(block->number_of_operands == ntokens-1) return 1;
	return 0;
	
}




