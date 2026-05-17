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
			return operand1 << 12;
		case 0b110:
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

uint32_t branch_comparator(){

}
