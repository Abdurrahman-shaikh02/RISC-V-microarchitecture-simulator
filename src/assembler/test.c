#include "assembler.h"

// two problems, its not handling only comments... nor is it handling newlines
// nor is it handling _ waale labels as operands
// all problems in the parser only LOL
static char * program[] = {
    // =========================
    // VALID INSTRUCTIONS
    // =========================
    "//hiiasid",		////
    "add r1, r2, r3",
    "sub r4, r5, r6",
    "and r7, r8, r9",
    "or r10, r11, r12",
    "xor r13, r14, r15",
    "sll r16, r17, r18",
    "srl r19, r20, r21",
    "sra r22, r23, r24",

    "addi r1, r2, 10",
    "andi r3, r4, 255",
    "ori r5, r6, -1",
    "xori r7, r8, 123",

    "slti r9, r10 -5",
    "sltiu r11, r12, 20",

    "slli r13, r14, 5",
    "srli r15, r16, 3",
    "srai r17, r18, 7",

    "lui r19, 0x12345",
    "auipc r20, 0xABCDe",

    "lw r1, 0(r2)",
    "lh r3, 4(r4)",
    "lb r5, 8(r6)",
    "lhu r7, 12(r8)",
    "lbu r9, 16(r10)",

    "sw r1, 0(r2)",
    "sh r3, 4(r4)",
    "sb r5, 8(r6)",

    "beq r1, r2, ll",
    "bne r3, r4, ll",
    "blt r5, r6, asd_asd",		////
    "bge r7, r8, sdj3_",		////
    "bltu r9, r10, asd_ads",		////
    "bgeu r11, r12, _34c",		////

    "jal r1, label",
    "jalr r2, r3, 0",

    // =========================
    // EDGE CASES (VALID)
    // =========================

    "addi r1, r2, 2047",
    "addi r3, r4, -2048",

    "slli r5, r6, 31",
    "srli r7, r8, 0",

    "lui r9, 0xFFFFF",	
    "auipc r10, 0x0",

    "beq r1, r1, label",

    "jal r0, asdj",
    "jal r0, asjdla",

    // =========================
    // INVALID INSTRUCTIONS
    // =========================

    "add r1, r2",
    "sub r1, r2, r33",
    "and r1, r2, r-1",

    "addi r1, r2, 5000",
    "andi r1, r2, -3000",

    "slli r1, r2, 40",
    "srli r1, r2, -1",

    "lui r1, 0x200000",

    "lw r1, r2",
    "sw r1, (r2)",

    "beq r1, r2",
    "bne r1, r2, 3",

    "jal r1",
    "jalr r1, r2",

    "foo r1, r2, r3",

    "addi r0, r0, 0",

    // =========================
    // SEMANTIC EDGE CASES
    // =========================

    "lw r1, 3(r2)",
    "sw r1, 5(r2)",

    "jalr r1, r2, 3",

    "//fsdfsdfsdf",
    "     // asdqwpeiqwe",
    "    //asdasda",
    "    /dsfsf",

    NULL
};

static char * program2[] = {

    // =========================
    // BASIC VALID
    // =========================

    "add r1,r2,r3",
    "add   r1 ,  r2 ,r3",
    "addi r1,r2,-10",
    "addi r1,r2,0x7ff",
    "addi r1,r2,-0x800",

    "lw r1,0(r2)",
    "sw r3,16(r4)",
    "lb r5,-8(r6)",

    // =========================
    // LABEL-LIKE OPERANDS
    // =========================

    "beq r1,r2,label",
    "beq r1,r2,label_1",
    "beq r1,r2,_label",
    "beq r1,r2,label_",
    "beq r1,r2,l_a_b_e_l",

    "jal r1,target",
    "jal r1,target_123",
    "jal r1,_start",

    // =========================
    // MIXED FORMATS
    // =========================

    "addi r1,r2,-0x1",
    "addi r1,r2,0x0",
    "addi r1,r2,-0",

    "lw r1,-0(r2)",
    "sw r1,0(r2)",

    // =========================
    // SPACING EDGE CASES
    // =========================

    "add r1 ,r2,r3",
    "add r1,r2 ,r3",
    "add r1 , r2,r3",
    "add r1 , r2 , r3",

    "lw r1, 0 ( r2 )",			//invalids ---> do not expect spaces in indexed addressing
    "sw r1 ,16( r2 )",

    // =========================
    // WEIRD BUT LEGAL TOKENS?
    // =========================

    "beq r1,r2,a1",
    "beq r1,r2,a_1",
    "beq r1,r2,a__b",
    "beq r1,r2,abc123",
    "beq r1,r2,abc_123_xyz",

    // =========================
    // BORDERLINE / TRICKY
    // =========================

    "addi r1,r2,--1",
    "addi r1,r2,- 1",
    "addi r1,r2,0x",
    "addi r1,r2,0xg",

    "lw r1,(r2)",
    "lw r1,()(r2)",
    "lw r1,0((r2))",

    "sw r1,0(r2",
    "sw r1,0r2)",

    // =========================
    // REGISTER EDGE CASES
    // =========================

    "add r01,r2,r3",
    "add r1,r02,r3",
    "add r1,r2,r003",

    "add r1,r2,r32",
    "add r1,r2,r-2",

    // =========================
    // HEX EDGE CASES
    // =========================

    "lui r1,0xfffff",
    "lui r1,0x00000",
    "lui r1,0x100000",

    "addi r1,r2,0x800",	
    "addi r1,r2,-0x801",

    // =========================
    // COMMA / TOKEN ERRORS
    // =========================

    "add r1 r2 r3",
    "add ,r1,r2,r3",
    "add r1,,r2,r3",
    "add r1,r2,",
    "add , , ,",

    // =========================
    // COMMENT INTERACTIONS
    // =========================

    "add r1,r2,r3 // normal",
    "add r1,r2,r3//tight",
    "add r1,r2,r3 / / broken",

    "// full line comment",
    "   // leading spaces",

    // =========================
    // MIXED CHAOS
    // =========================

    "beq r1 , r2 , label_123 // comment",
    "lw r1 , -16 ( r2 )",
    "addi r1 , r2 , -0x1f",

    "jal r1 , _entry_point",
    "jal r1 , __",
    "jal r1 , ___label___",
    "skfjskdjfk:",
    "skfjsasd:",
    "skfjskdjfk:",
    "skfjsk324:",

    NULL
};

static char * program3[] = {
	"sub r1, r2, r3",
	"addi r1, r2, 7",
	"lui r31, 45",
	"test: ",		// let label address be 40
	"sb r31, 45(r7)",
	"sb r31, -45(r7)",
	//"jal r1, test",
	//"beq r1, r2, test",
	NULL
};

int main(){
	char * s = malloc(128);
	char ** ic = program2;

	char ** ret_array = (char **)malloc(sizeof(char *) * MAX_TOKENS_IN_A_LINE);

	//allocate memory for each token string
	for(int i = 0; i < MAX_TOKENS_IN_A_LINE; i++){
		ret_array[i] = (char *)malloc(sizeof(char) * MAX_TOKEN_CHAR);
	}

	while(*ic != NULL){
		//initialize all strings as 0
		for(int i = 0; i < MAX_TOKENS_IN_A_LINE; i++){
			char * l = ret_array[i];
			for(int j = 0; j < MAX_TOKEN_CHAR; j++){
				l[j] = 0;
			}
		}


		strcpy(s, *ic);

		int t = get_tokens(s, ret_array, MAX_TOKENS_IN_A_LINE, MAX_TOKEN_CHAR);
		if(t == -1){
			printf("Invalid instruction format\n");
			ic++;
			continue;
		}
		if(t == 10){
			printf("comment\n");
			ic++;
			continue;
		}

		if(t == 0){

			if(add_symbol(ret_array[0], 40) == -1){
				printf("symbol failed\n");
				ic++;
				continue;
			}

			printf("%d -- ", get_address(ret_array[0]));

			printf("label --> ");
			ic++;

			printf("%s\n", ret_array[0]);
			continue;
		}
		int v = check_syntax(ret_array, t, 1);
		if(!v){
			ic++;
			continue;
		}

		int u = check_semantics(ret_array, 1);
		if(!u){
			ic++;
			continue;
		}


		printf("%d : ", t);

		for(int i = 0; i < 4; i++){
			printf("%s ", ret_array[i]);

		}
		
		//printf("%032b\n", get_opcode(ret_array, 0));

		ic++;
	}

}
