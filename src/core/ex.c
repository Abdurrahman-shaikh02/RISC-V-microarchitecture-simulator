#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"
#include "alu.h"

static int PAUSE = 0;

void ex_stage(){
	log_info("EX stage initiating.");

	ex_control control = id_ex.cs_ex;

	if(control.PAUSE){
		log_info("Breakpoint");
		PAUSE = 1;
		return;
	}

	uint32_t operand1;
	if(control.source1 == 0b00000001){
		operand1 = pc;
		log_debug("r1 = pc");
	}else if(control.source1 == 0b00000010){
		operand1 = id_ex.R1;
		log_debug("r1 = rs1");
	}else if(control.source1 == 0b00000100){
		operand1 = id_ex.R2;
		log_debug("r1 = rs2");
	}

	uint32_t operand2;
	if(control.source2 == 0){
		operand2 = id_ex.R2;
		log_debug("r2 = rs2");
	}else if(control.source2 == 1){
		operand2 = id_ex.imm;
		log_debug("r2 = imm");
	}

	uint32_t result = 0;

	switch(control.type){
		case 0b00000001:
			//arithmetic
			log_debug("arithmetic op");
			result = arithmetic_unit(operand1, operand2, control.arithmetic_opcode);
			break;
		case 0b00000010:
			//logic
			log_debug("logical op");
			result = logical_unit(operand1, operand2, control.logic_opcode, control.sign);
			break;
		case 0b00000100:
			//less than
			log_debug("less than compare");
			result = less_than_comparator(operand1, operand2, control.sign);
			break;
		case 0b00001000:
			//branch compare
			log_debug("branch compare");
			//handle result, handle the branch taken flag as well....
			break;
		default:
			log_fatal("invalid execute stage");
			exit(1);
	}

	//fill up the ex_ma register
	
	ex_ma.result = result;
	ex_ma.cs_ma = id_ex.cs_ma;
	ex_ma.cs_wb = id_ex.cs_wb;
	ex_ma.R1 = id_ex.R1;
	ex_ma.nrd = id_ex.nrd;
	ex_ma.PC_next = id_ex.PC_next;



}
