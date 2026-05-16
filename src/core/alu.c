#include "header.h"
#include "alu.h"

uint32_t arithmetic_unit(uint32_t operand1, uint32_t operand2, uint32_t opcode){
	switch(opcode){
		case 0b00000000:
			//add
			return operand1 + operand2;
		case 0b00000001:
			//sub
			return operand1 - operand2;
		default:
			log_fatal("invalid alu opcode");
			exit(1);
	}
}

uint32_t logical_unit(uint32_t operand1, uint32_t operand2, uint32_t opcode, uint8_t sign){
	switch(opcode){
		case 0b00000001:
			//and
			return operand1 & operand2;
		case 0b00000010:
			//or
			return operand1 | operand2;
		case 0b00000100:
			//xor
			return (operand1 ^ operand2);
		case 0b00001000:
			//left shift
			return operand1 << operand2;
		case 0b00010000:
			//right shift
			if(sign == 0){
				return operand1 >> operand2;
			}else{
				return (uint32_t)(((int32_t)operand1) >> operand2);
			}
		case 0b00100000:
			//left shift 12
			return operand1 << 12;
		case 0b01000000:
			//left shift 12 and add to pc(pass as operand two)
			return (operand1 << 12) + operand2;
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
	}else{
		if((int32_t)operand1 < (int32_t)operand2){
			return 1;
		}else{
			return 0;
		}
	}
}

uint32_t branch_comparator(){

}
