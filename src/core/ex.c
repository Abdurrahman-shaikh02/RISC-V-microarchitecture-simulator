#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"
#include "alu.h"


void ex_stage(){
	log_info("EX stage initiating.");

	ex_control control = id_ex.cs_ex;

	if(control.PAUSE){
		log_info("Breakpoint");
		PAUSE = 1;
		ex_ma.ins = id_ex.ins;		//need to make sure instruction is correct for visuals
		return;
	}

	uint32_t operand1;
	switch(control.source1){
		case 0b00:
			operand1 = id_ex.PC;
			log_debug("r1 = pc");
			break;
		case 0b01:
			operand1 = id_ex.R1;
			log_debug("r1 = rs1");
			break;
	/*	case 0b10:
			operand1 = id_ex.R2;
			log_debug("r1 = rs2");
			break;
	*/
		default:
			log_fatal("Invalid source 1.");
			exit(1);
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
		case 0b00:
			//arithmetic
			log_debug("arithmetic op");
			result = arithmetic_unit(operand1, operand2, control.arithmetic_opcode);
			break;
		case 0b01:
			//logic
			log_debug("logical op");
			result = logical_unit(operand1, operand2, control.logic_opcode, control.sign);
			break;
		case 0b10:
			//less than
			log_debug("less than compare");
			result = less_than_comparator(operand1, operand2, control.sign);
			break;
		case 0b11:
			//branch compare
			//comapre, change pc, set a global stall flag(to flush the pipe, later used by branch pred)
			//set result = pc_next (for jump instructions...) regardless of branch or jump
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
	ex_ma.R2 = id_ex.R2;
	ex_ma.nrd = id_ex.nrd;
	ex_ma.PC_next = id_ex.PC_next;
	ex_ma.ins = id_ex.ins;




}
