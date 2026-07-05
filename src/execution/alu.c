#include "header.h"
#include "alu.h"


uint32_t arithmetic_unit(uint32_t operand1, uint32_t operand2, uint32_t opcode){
	switch(opcode){
		case 0b0:
			//add
			return operand1 + operand2;
		case 0b1:
			//sub
			return operand1 - operand2;
		default:
			log_fatal("invalid alu opcode");
			exit(1);
	}
}

uint32_t logical_unit(uint32_t operand1, uint32_t operand2, uint32_t opcode, uint8_t sign){
	switch(opcode){
		case 0b000:
			//and
			return operand1 & operand2;
		case 0b001:
			//or
			return operand1 | operand2;
		case 0b010:
			//xor
			return (operand1 ^ operand2);
		case 0b011:
			//left shift
			return operand1 << (operand2 & 0x0000001F);	//risc v isa uses only lower 5 bits... this masking behaviour is defined in the isa
		case 0b100:
			//right shift
			if(sign == 0){
				return operand1 >> (operand2 & 0x0000001F);
			}else{
				return (uint32_t)(((int32_t)operand1) >> (operand2 & 0x0000001F));	//again the masking is defined by the isa
			}
		case 0b101:
			//left shift 12
			return operand2 << 12;
		case 0b110:
			//left shift 12 and add to pc(pass as operand two)
			return (operand2 << 12) + operand1;
		default:
			log_fatal("invalid logical unit opcode");
			exit(1);
		
	}
}

uint32_t less_than_comparator(uint32_t operand1, uint32_t operand2, uint8_t sign){
	if(sign == 0){
		if(operand1 < operand2){
			return 1;
		}else{
			return 0;
		}
	}else if(sign == 1){
		if((int32_t)operand1 < (int32_t)operand2){
			return 1;
		}else{
			return 0;
		}
	}
	log_fatal("less than comparator failed.");
	exit(1);
}

uint32_t branch_comparator(uint32_t operand1, uint32_t operand2, uint8_t branch_opcode, uint8_t sign){

	switch(branch_opcode){
		case 0b000:
			//beq
			if(operand1 == operand2){
				return 1;
			}
			return 0;
		case 0b001:
			//bne
			if(operand1 != operand2){
				return 1;
			}
			return 0;
		case 0b010:
			//blt
			if(sign == 0){
				if(operand1 < operand2){
					return 1;
				}
			}else{
				if((int32_t)operand1 < (int32_t)operand2){
					return 1;
				}
			}
			return 0;
		case 0b011:
			//bge
			if(sign == 0){
				if(operand1 >= operand2){
					return 1;
				}
			}else{
				if((int32_t)operand1 >= (int32_t)operand2){
					return 1;
				}
			}
			return 0;
		default:
			log_fatal("Invalid branch opcode.");
			exit(1);
	}

}


//NEED TO HANDLE SIGN EXTENSION
uint32_t generate_immediate(char format, uint32_t ir){
	uint32_t imm = 0;

	switch(format){
		case 0:
			//its a bubble
			log_debug("no immediate was generated for a bubble");
			return 0;
		case 'r':
			log_debug("no immediate was generated");
			return 0;
		case 'i':
			imm = (int32_t)(ir & 0xFFF00000) >> 20;
			log_debug("immediate generated for i format");
			break;
		case 's':
			imm = ir & 0xFE000000;
			imm = imm | ((ir & 0x00000F80) << 13);
			imm = ((int32_t)imm) >> 20;
			log_debug("immediate generated for s format");
			break;
		case 'b':
			imm = (ir & 0x00000F00) >> 8;
			imm = imm | ((ir & 0x7E000000) >> 21);
			imm = imm | ((ir & 0x80) << 3);
			imm = imm | (((int32_t)(ir & 0x80000000)) >> 20);
			imm = imm << 1;		//left shift by one if its a branch/jump
			break;
		case 'u':
			imm = (int32_t)(ir & 0xFFFFF000) >> 12;
			log_debug("immediate generated for u format");
			break;
		case 'j':
			imm = (ir & 0x7FE00000) >> 21;
			imm = imm | ((ir & 0x00100000) >> 10);
			imm = imm | ((ir & 0x000FF000) >> 1);
			imm = imm | (((int32_t)(ir & 0x80000000)) >> 12);
			imm = imm << 1;		//left shift by one if its a jump/branch
			log_debug("immediate generated for j format");
			break;
		default:
			log_fatal("invalid format for immediate generation");
			exit(1);
	}

	return imm;
}

