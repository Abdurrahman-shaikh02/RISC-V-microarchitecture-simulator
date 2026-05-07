#include "assembler.h"

int main(int argc, char * argv[]){
    int verbose = 0;
    const char * path;
    const char * out_path = "output.txt";

    if(argc == 4){
	if(!strcmp(*(argv + 1), "-v")){
	    verbose = 1;
	    path = *(argv + 2);
	    out_path = *(argv + 3);
	}
	else{
	    printf("Invalid arguments1\n");
	    return -1;
	}
    }else if(argc == 3){
	if(!strcmp(*(argv + 1), "-v")){
	    verbose = 1;
	    path = *(argv + 2);
	}
	else{
	    path = *(argv + 1);
	    out_path = *(argv + 2);
	    return -1;
	}
    }else if(argc == 2){
	path = *(argv + 1);
    }else{
	printf("Invalid arguments\n");
	return -1;
    }

    FILE * source = fopen(path, "r");
    if(!source){
	printf("File not found\n");
	return -1;
    }

    // get tokens needs a dynamic char[] 
    char * line = (char * )malloc(256 * sizeof(char));
    char ** parsed_line = (char **)malloc(sizeof(char *) * MAX_TOKENS_IN_A_LINE);
    //intialisation of parsed_line
    for(int i = 0; i < MAX_TOKENS_IN_A_LINE; i++){
	parsed_line[i] = (char *)malloc(sizeof(char) * MAX_TOKEN_CHAR);
    }
    int instruction_number = 0;
    int line_count = 0;
    while(fgets(line, 256, source) != NULL){
	line_count++;
	//-------------------------------------------------handle an empty line with only white spaces here--------------------------
	int white = 1;
	char * s = line;
	while(*s){
	    if(!isspace(*s)){
		white = 0;
		break;
	    }
	    s++;
	}
	if(white) continue;


	int parser = get_tokens(line, parsed_line, MAX_TOKENS_IN_A_LINE, MAX_TOKEN_CHAR);
	if(parser == -1){
	    printf("Invalid parsing at line number : %d\n", line_count);
	    log_fatal("Invalid parsing encountered");
	    return -1;
	}else if(parser == 10){
	    log_info("Handled a full line comment");
	    continue;
	}else if(parser == 0){
	    log_info("Handling a label...");
	    
	    if(add_symbol(parsed_line[0], (instruction_number*4)) == -1){	//since we are not incrementing the instruction_number for a label... the instruction_number during the iteration of the label is the instruction_number of the next instruction....
		log_fatal("Couldnt add symbol to table.");
		return -1;
	    }else{
		log_info("Added symbol to table");
		continue; }
	}

	if(!check_syntax(parsed_line, parser, 1)){
	    printf("Syntax error at line number : %d\n", line_count);
	    log_fatal("Invalid syntax encountered");
	    return -1;
	}else{
	    log_info("Valid syntax...");
	}


	if(!check_semantics(parsed_line, 1)){
	    printf("Semantics error at line number : %d\n", line_count);
	    log_fatal("Invalid semantics encountered");
	    return -1;
	}else{
	    log_info("Valid semantics...");
	}
	
	instruction_number++;
	
    }

    // preparing for second pass
    int address = 0;
    fclose(source);
    
    source = fopen(path, "r");
    FILE * object = fopen(out_path, "w");
    if(!object) return -1;

    while(fgets(line, 256, source)){
	//-------------------------------------------------handle an empty line with only white spaces here--------------------------
	int white = 1;
	char * s = line;
	while(*s){
	    if(!isspace(*s)){
		white = 0;
		break;
	    }
	    s++;
	}
	if(white) continue;


	int parser = get_tokens(line, parsed_line, MAX_TOKENS_IN_A_LINE, MAX_TOKEN_CHAR);
	if(parser == 10){
	    log_info("Handled a full line comment");
	    continue;
	}else if(parser == 0){
	    log_info("Handling a label again...");
	    if(verbose) fprintf(object, "%s", line);
	    continue;
	}
	

	int opcode = get_opcode(parsed_line, address);
	
	if(!opcode){
	    printf("Opcode couldnt be generated.\n");
	    log_fatal("Some error in opcode generation...");
	    return -1;
	}
	
	if(!verbose){
	    fprintf(object, "%08x\n", opcode);
	}else if(verbose){
	    fprintf(object, "[%d] %#08x   %08x   %s", address/4, address, opcode, line);
	}

	address = address + 4;

	
	
    }


    fclose(source);
    fclose(object);
    for(int i = 0; i < MAX_TOKENS_IN_A_LINE; i++){
	free(parsed_line[i]);
    }
    free(parsed_line);
    free(line);
}
