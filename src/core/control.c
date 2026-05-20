#include "header.h"
#include "control.h"

static const int i_table_length = 41;
static const i_block i_table[] = {
	// R-TYPE
	{"add", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b000, 0b0000000},
	{"sub", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b000, 0b0100000},
	{"sll", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b001, 0b0000000},
	{"slt", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b010, 0b0000000},
	{"sltu",3, 'r', 'r', 'r', 'r', 0b0110011, 0b011, 0b0000000},
	{"xor", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b100, 0b0000000},
	{"srl", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b101, 0b0000000},
	{"sra", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b101, 0b0100000},
	{"or",  3, 'r', 'r', 'r', 'r', 0b0110011, 0b110, 0b0000000},
	{"and", 3, 'r', 'r', 'r', 'r', 0b0110011, 0b111, 0b0000000},

	// I-TYPE (ARITH)
	{"addi", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b000, 0},
	{"slti", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b010, 0},
	{"sltiu",3, 'r', 'r', 'i', 'i', 0b0010011, 0b011, 0},
	{"xori", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b100, 0},
	{"ori",  3, 'r', 'r', 'i', 'i', 0b0010011, 0b110, 0},
	{"andi", 3, 'r', 'r', 'i', 'i', 0b0010011, 0b111, 0},

	// SHIFT IMM (SPECIAL I)
	{"slli", 3, 'r', 'r', 's', 'i', 0b0010011, 0b001, 0b0000000},	// s : 5 bit immediate
	{"srli", 3, 'r', 'r', 's', 'i', 0b0010011, 0b101, 0b0000000},
	{"srai", 3, 'r', 'r', 's', 'i', 0b0010011, 0b101, 0b0100000},

	// U-TYPE
	{"lui",   2, 'r', 'u', 0, 'u', 0b0110111, 0, 0},		// u : 20 bit immediate
	{"auipc", 2, 'r', 'u', 0, 'u', 0b0010111, 0, 0},

	// J-TYPE
	{"jal", 2, 'r', 'l', 0, 'j', 0b1101111, 0, 0},			// l : label

	// I-TYPE (JUMP REG)
	{"jalr", 3, 'r', 'r', 'i', 'i', 0b1100111, 0b000, 0},		//can you confirm this is it supposed to be a label ?

	// B-TYPE
	{"beq",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b000, 0},
	{"bne",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b001, 0},
	{"blt",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b100, 0},
	{"bge",  3, 'r', 'r', 'l', 'b', 0b1100011, 0b101, 0},
	{"bltu", 3, 'r', 'r', 'l', 'b', 0b1100011, 0b110, 0},
	{"bgeu", 3, 'r', 'r', 'l', 'b', 0b1100011, 0b111, 0},

	// LOADS (I-TYPE)
	{"lb",  2, 'r', 'a', 0, 'i', 0b0000011, 0b000, 0},		// a : indexed addressing mode
	{"lh",  2, 'r', 'a', 0, 'i', 0b0000011, 0b001, 0},
	{"lw",  2, 'r', 'a', 0, 'i', 0b0000011, 0b010, 0},
	{"lbu", 2, 'r', 'a', 0, 'i', 0b0000011, 0b100, 0},
	{"lhu", 2, 'r', 'a', 0, 'i', 0b0000011, 0b101, 0},

	// STORES (S-TYPE)
	{"sb", 2, 'r', 'a', 0, 's', 0b0100011, 0b000, 0},
	{"sh", 2, 'r', 'a', 0, 's', 0b0100011, 0b001, 0},
	{"sw", 2, 'r', 'a', 0, 's', 0b0100011, 0b010, 0},

	
	// SYSTEM
	{"ecall",  0, 0, 0, 0, 'i', 0b1110011, 0b000, 0},		//execute as a bubble
	{"ebreak", 0, 0, 0, 0, 'i', 0b1110011, 0b000, 1},		//execute as a breakpoint, causing execution to pause... after its EX stage

	// FENCE
	{"fence", 0, 0, 0, 0, 'i', 0b0001111, 0b000, 0},		//execute as a bubble
	

	// NOP (pseudo but useful)
	{"nop", 0, 0, 0, 0, 'i', 0b0010011, 0b000, 0}
};

/*
const i_block * find_block(uint32_t opcode, int32_t funct3, int32_t funct7){
	if(funct3 == -1 && funct7 != -1){
		log_fatal("invalid block search");
		exit(1);
	}

	for(int i = 0; i < i_table_length; i++){
		if(i_table[i].opcode == opcode){
			if(funct3 == -1){
				return (i_table + i);
			}else{
				if(i_table[i].funct3 == funct3){
					if(funct7 == -1){
						return (i_table + i);
					}else{
						if(i_table[i].funct7 == funct7){
							return (i_table + i);
						}
					}
				}
			}
		}
	}

	return NULL;
}
*/

char get_format(uint32_t opcode){
	for(int i = 0; i < i_table_length; i++){
		if(i_table[i].opcode == opcode) return (i_table[i].format);
	}

	return 0;
}

cu_output read_r_format(uint32_t ir){
	uint32_t opcode = (ir & 0x0000007F);
	int32_t funct3 = (ir & 0x00007000) >> 12;
	int32_t funct7 = (ir & 0xFE000000) >> 25;

	/*
	const i_block * block = find_block(opcode, funct3, funct7);
	if(block == NULL){
		log_fatal("invalid opcode");
		exit(1);
	}
	*/

	switch(funct3){
		case 0b000:
			//add or sub...
			switch(funct7){
				case 0b0000000:
					//add
					return (cu_output){'r', {1, 0, 1, 0, 0, 0, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b0100000:
					//sub
					return (cu_output){'r', {1, 0, 1, 0, 1, 0, 0, 0}, {0, 0, 0, 0}, {1}};
				default:
					log_fatal("error in funct7 in r type");
					exit(1);
			}
		case 0b001: 
			//sll
			return (cu_output){'r', {1, 0, 0, 0b01, 0, 0b011, 0, 0}, {0, 0, 0, 0}, {1}};
		case 0b010:
			//slt
			return (cu_output){'r', {1, 0, 1, 0b10, 0, 0, 0, 0}, {0, 0, 0, 0}, {1}};
		case 0b011:
			//sltu
			return (cu_output){'r', {1, 0, 0, 0b10, 0, 0, 0, 0}, {0, 0, 0, 0}, {1}};
		case 0b100:
			//xor
			return (cu_output){'r', {1, 0, 0, 0b01, 0, 0b010, 0, 0}, {0, 0, 0, 0}, {1}};
		case 0b101:
			//srl or sra
			switch(funct7){
				case 0b0000000:
					//srl
					return (cu_output){'r', {1, 0, 0, 0b01, 0, 0b100, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b0100000:
					//sra
					return (cu_output){'r', {1, 0, 1, 0b01, 0, 0b100, 0, 0}, {0, 0, 0, 0}, {1}};
				default:
					log_fatal("error in funct7 in r type");
					exit(1);
			}
		case 0b110:
			//or
			return (cu_output){'r', {1, 0, 0, 0b01, 0, 0b001, 0, 0}, {0, 0, 0, 0}, {1}};
		case 0b111:
			//and
			return (cu_output){'r', {1, 0, 0, 0b01, 0, 0b000, 0, 0}, {0, 0, 0, 0}, {1}};
		default:
			log_fatal("error in funct3");
			exit(1);
	}

}

cu_output read_i_format(uint32_t ir){
	uint32_t opcode = (ir & 0x0000007F);
	int32_t funct3 = (ir & 0x00007000) >> 12;
	int32_t funct7 = (ir & 0xFE000000) >> 25;

	switch(opcode){
		case 0b0010011:
			//arithmetics, special arithmetics
			switch(funct3){
				case 0b000:
					//addi
					return (cu_output){'i', {1, 1, 1, 0, 0, 0, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b010:
					//slti
					return (cu_output){'i', {1, 1, 1, 0b10, 0, 0, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b011:
					//sltiu
					return (cu_output){'i', {1, 1, 0, 0b10, 0, 0, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b100:
					//xori
					return (cu_output){'i', {1, 1, 0, 0b01, 0, 0b010, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b110:
					//ori
					return (cu_output){'i', {1, 1, 0, 0b01, 0, 0b001, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b111:
					//andi
					return (cu_output){'i', {1, 1, 0, 0b01, 0, 0b000, 0, 0}, {0, 0, 0, 0}, {1}};
				case 0b001:
					switch(funct7){
						case 0b0000000:
							//slli
							return (cu_output){'i', {1, 1, 0, 0b01, 0, 0b011, 0, 0}, {0, 0, 0, 0}, {1}};
						default:
							log_fatal("error in funct 7 in i type");
							exit(1);
					}
				case 0b101:
					switch(funct7){
						case 0b0000000:
							//srli
							return (cu_output){'i', {1, 1, 0, 0b01, 0, 0b100, 0, 0}, {0, 0, 0, 0}, {1}};
						case 0b0100000:
							//srai
							return (cu_output){'i', {1, 1, 1, 0b01, 0, 0b100, 0, 0}, {0, 0, 0, 0}, {1}};
						default:
							log_fatal("error in funct 7 in i type");
							exit(1);
					}
			}
		case 0b1100111:
			//jalr
		case 0b0000011:
			//loads
			switch(funct3){
				case 0b000:
					//lb
					return (cu_output){'i', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b0000, 0, 0}, {1}};
				case 0b001:
					//lh
					return (cu_output){'i', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b0001, 0, 0}, {1}};
				case 0b010:
					//lw
					return (cu_output){'i', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b0010, 0, 0}, {1}};
				case 0b100:
					//lbu
					return (cu_output){'i', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b0011, 0, 0}, {1}};
				case 0b101:
					//lhu
					return (cu_output){'i', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b0100, 0, 0}, {1}};
				default:
					log_fatal("error in funct3");
					exit(1);
			}
		case 0b1110011:
			//system calls
			switch(funct7){
				case 0:
					//ecall
					return (cu_output){0, {0,0,0,0,0,0,0,0b10}, {0,0,0,0}, {0}};
				case 1:
					//ebreak
					return (cu_output){0, {0,0,0,0,0,0,0,1}, {0,0,0,0}, {0}};
				default:
					log_fatal("error funct 7 in i type -");
					exit(1);
			}
		case 0b0001111:
			//fence
			return (cu_output){0, {0,0,0,0,0,0,0,0}, {0,0,0,0}, {0}};
		default:
			log_fatal("invalid opcode for i type encoding");
			exit(1);
	}

}

cu_output read_s_format(uint32_t ir){
	uint32_t opcode = (ir & 0x0000007F);
	int32_t funct3 = (ir & 0x00007000) >> 12;
	int32_t funct7 = (ir & 0xFE000000) >> 25;


	switch(funct3){
		case 0b000:
			//sb
			return (cu_output){'s', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b1000, 0, 0}, {0}};
		case 0b001:
			//sh
			return (cu_output){'s', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b1001, 0, 0}, {0}};
		case 0b010:
			//sw
			return (cu_output){'s', {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0b1010, 0, 0}, {0}};
		default:
			log_fatal("error in funct3 in s type.");
			exit(1);
	}
}

cu_output read_b_format(uint32_t ir){
	uint32_t opcode = (ir & 0x0000007F);
	int32_t funct3 = (ir & 0x00007000) >> 12;
	int32_t funct7 = (ir & 0xFE000000) >> 25;

	switch(funct3){
		case 0b000:
			//beq
		case 0b001:
			//bne
		case 0b100:
			//blt
		case 0b101:
			//bge
		case 0b110:
			//bltu
		case 0b111:
			//bgeu
		default:
			log_fatal("error in funct3 in b type.");
			exit(1);
	}

}

cu_output read_u_format(uint32_t ir){
	uint32_t opcode = (ir & 0x0000007F);

	switch(opcode){
		case 0b0110111:
			//lui
			return (cu_output){'u', {0, 1, 0, 0b01, 0, 0b101, 0, 0}, {0,0,0,0}, {1}};
		case 0b0010111:
			//auipc
			return (cu_output){'u', {0, 1, 0, 0b01, 0, 0b110, 0, 0}, {0,0,0,0}, {1}};
		default:
			log_fatal("error in opcode in u type.");
			exit(1);
	}

}

cu_output read_j_format(uint32_t ir){
	uint32_t opcode = (ir & 0x0000007F);

	switch(opcode){
		case 0b1101111:
			//jal
		default:
			log_fatal("error in opcode in j type.");
			exit(1);
	}
}
// read different format (different functions for each format) generate control signals and store them directly in a new struct defined in control.h
// copy that struct in the id_stage function !!

cu_output decode(uint32_t ir) {
	uint32_t opcode = ir & 0x7F;
	if(opcode == 0){
		//its a bubble
		return (cu_output){0, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0 ,0 ,0}, {0}};
	}
	char format = get_format(opcode);
	if(format == 0){
		log_fatal("invalid opcode, couldnt find a format");
		exit(1);
	}

	switch(format){
		case 'r':
			log_debug("dispatch to r format");
			return read_r_format(ir);
		case 'i':
			log_debug("dispatch to i format");
			return read_i_format(ir);
		case 's':
			log_debug("dispatch to s format");
			return read_s_format(ir);
		case 'b':
			log_debug("dispatch to b format");
			return read_b_format(ir);
		case 'u':
			log_debug("dispatch to u format");
			return read_u_format(ir);
		case 'j':
			log_debug("dispatch to j format");
			return read_j_format(ir);
	}

	log_fatal("decode failed after switch case.. bruh how ?");
	exit(1);


}
