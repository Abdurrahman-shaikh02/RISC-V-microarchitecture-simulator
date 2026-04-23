#include "assembler.h"
#include <ctype.h>

// this parser takes in one line and puts tokens in a passed array of strings, the caller of get_tokens must handle allocation of both arrays
// also fixes case -- converts everything to lowercase except labels
// this simple parser will give error if not the following formats,
// <op>
// <op> <reg>
// <op> <reg>,<reg>
// <op> <reg>,<reg>,<reg>
// label:
// //comments ---> comments may also be on a line with an operation/label
//
// this parser handle brackes (), and - in such a way that it gives error if they are a part of the operation name its an error, but when they are a part of any argument they will be view as alphanumerics
// that means it will parse strings like --- add (,()), ()	--- > handle these in the semantic analyzer

int parse_label(char * s, char ** ret_array, int max_tokens, int max_chars){
	//returns 0 for an error, 1 for a success
	char * t = ret_array[0];

	while(isspace(*s)){
		s++;
	}

	while(isalnum(*s) || *s == '_'){
		*t = tolower(*s);
		t++;
		s++;
	}
	*t = '\0';

	if(isspace(*s)) return 0;
	if(*s == ':'){
		s++;
		while(isspace(*s)){
			s++;
		}
		if(*s) return 0;
	}
	return 1;
	
}

int parse_operation(char * s, char ** ret_array, int max_tokens, int max_chars){
	// returns 0 for a syntax error, no of tokens read for a success
	char * t;
	int i = 0;

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
	if(t == ret_array[i]) return 0;	//no operation name
	*t = '\0';

	//if the operation name immediately precedes a comma or any bracket or a minus or an underscore theres an error 
	if(*s == ',' || *s == '(' || *s == ')' || *s == '-' || *s == '_'){
		return 0;
	}

	//if theres no comma or brackets, there has to be a space or a null... null will be checked later... skip the spaces
	while(isspace(*s)){
		s++;
	}

	if(*s) i++;  //if null then dont increment i and go forwards , which will reach end return statement
	//if not null then increment i and enter loop


	while(*s){
		if(i>=max_tokens){
			i = -1;
			break;
		}

		while(isspace(*s)){
			s++;
		}
		
		t = ret_array[i];
		while(isalnum(*s) || *s == '(' || *s == ')' || *s == '-' || *s == '_'){
			*t = tolower(*s);
			s++;
			t++;
		}
		if(t == ret_array[i]) return 0;
		*t = '\0';

		while(isspace(*s)){
			s++;
		}

		if(*s == ','){
			i++;
			s++;
			if(!*s) return 0;		//add s1,     --->     comma and the string ends the full while loop , hence the check
		}else if(isalnum(*s) || *s == '(' || *s == ')' || *s == '-' || *s == '_'){
			return 0;
		}else{
			break;
		}
	}

	if(i == -1) return 0;
	return i + 1;

}

char * remove_comments(char * s){
	// returns NULL if theres an error , user must check for error
	char * t = s;
	while(*t){
		if(*t == '/'){
			if(*(t+1) == '/'){
				*t = '\n';
				*(t+1) = 0;
				return s;
			}
			else{
				log_debug("// not found.");
				return NULL;
			}
		}
		t++;
	}
	return s;
}


int is_full_line_comment(char * s){
	while(isspace(*s)) s++;
	if(*s == '/') if(*(s+1) == '/') return 1;
	return 0;

}


int get_tokens (char * s, char ** ret_array, int max_tokens, int max_chars){	//max tokens in a line , max chars in each token
	// returns -1 for error, number of tokens for an operation, 0 for a label, 10 for a full comment--> to be ignored
	// needs a dynamic s... it makes changes to it, to ignore comments...
	if(is_full_line_comment(s)){
		log_info("Processed a full line comment.");
		return 10;
	}
	
	s = remove_comments(s);

	if(!s){
		log_error("Comment handling failed.");
		return -1; // check for errors
	}

	int i = 0; // keeps count in the ret_array
	int label = 0;
	int underscore = 0;
	int bracket = 0;
	int negative = 0;
	char * t = s;

	//check for characters
	while(*t){
		if(!(isspace(*t) || isalnum(*t) || *t == ',' || *t == ':' || *t == '_' || *t == '(' || *t == ')' || *t == '-')) return -1;	//invalid character
		if(*t == ':') label = 1;
		if(*t == '_') underscore = 1;
		if(*t == ')' || *t == '(') bracket = 1;
		if(*t == '-') negative = 1;
		t++;
	}

//---------------------------------theres gotta be a better way to do this--------------------------------------------------------
	// if you see brackets, then it cant be a label ie no colon or underscore
	if(bracket){
		if(label) return -1;
		if(underscore) return -1;
	}
	// if you see an underscore and no colon then its an error
	if(underscore){
		//if(!label) return -1;
		if(negative) return -1;
		if(bracket) return -1;
	}

	if(negative){
		if(label) return -1;
		if(underscore) return -1;
	}

	if(label){
		if(negative) return -1;
		if(bracket) return -1;
	}

	log_info("Characters are valid.");
//---------------------------------------------------------------------------------------------------------------------------------

	if(label){
		log_info("Parsing a label.");

		int success = parse_label(s, ret_array, max_tokens, max_chars);
		if(!success){
			log_error("Label parsing failed.");
			return -1;
		}
		else{
			log_info("Label parsing succeeded.");
			return 0;
		}
	}else{
		log_info("Parsing an operation.");

		int success = parse_operation(s, ret_array, max_tokens, max_chars);
		if(!success){
			log_error("Operation parsing failed.");
			return -1;
		}
		else{
			log_info("Operation parsing succeeded.");
			return success;
		}
	}
}



