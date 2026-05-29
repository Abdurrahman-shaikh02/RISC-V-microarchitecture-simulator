#include "header.h"
#include "branch_prediction.h"
#include "alu.h"

int predict_not_taken(uint32_t ir){
	return 0;
}

int predict_taken(uint32_t ir){
	return 1;
}

int predict_based_on_offset(uint32_t ir){
	uint32_t opcode = ir & 0x7F;
	
	//predict taken if the offset is -ve ... and positive otherwise
	int32_t imm = generate_immediate('b', ir);

	if(imm >= 0){
		return 0;
	}else{
		return 1;
	}

}


