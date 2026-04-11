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
int get_number_of_operands(char * s);	//table look up --- linear search
int check_syntax(char ** array, int ntokens);	//take operation name, and number of tokens read and 0 for error, 1 for OK
