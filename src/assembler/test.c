#include "./../../include/assembler/assembler.h"

int main(){
	char * s = malloc(64);
	strcpy(s, "lb x1, 43(x5)");

	
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
	

	int t = get_tokens(s, ret_array, MAX_TOKENS_IN_A_LINE, MAX_TOKEN_CHAR);
	if(t == -1) return 0;

	int v = check_syntax(ret_array, t);
	if(!v) return 0;


	printf("%d\n", t);

	for(int i = 0; i < 4; i++){
		printf("%s\n", ret_array[i]);

	}
	
}
