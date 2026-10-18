#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "internal_memory.h"
#include "alu.h"
#include "branch_prediction.h"
#include "forwarding.h"


void ex_stage(){
	log_info("EX stage initiating.");

	ex_control control = id_ex.cs_ex;

	if(control.SYSTEM_CALL == 1){
		log_info("Breakpoint");
		PAUSE = 1;
		ex_ma.ins = id_ex.ins;		//need to make sure instruction is correct for visuals
		return;
	}else if(control.SYSTEM_CALL == 0b10){
		log_info("System call");
		TRAP = 1;
		ex_ma.ins = id_ex.ins;		//need to make sure instruction is correct for visuals
		return;
	}

	uint32_t operand1 = 0;
	switch(control.source1){
		case 0b00:
			operand1 = id_ex.PC;
			log_debug("r1 = pc");
			break;
		case 0b01:
			log_info("Checking for data hazards in operand1 (ex forward)");
			operand1 = forward_ex(id_ex.R1, id_ex.nrs1);

			log_debug("r1 = rs1");
			break;
		default:
			log_fatal("Invalid source 1.");
			exit(1);
	}

	uint32_t operand2 = 0;
	switch(control.source2){
		case 0:
			log_info("Checking for data hazards in operand2 (ex_forward)");
			operand2 = forward_ex(id_ex.R2, id_ex.nrs2);

			log_debug("r2 = rs2");
			break;
		case 1:
			operand2 = id_ex.imm;
			log_debug("r2 = imm");
			break;
		default:
			log_fatal("Invalid source 2.");
			exit(1);
	}

	uint32_t result = 0;
	uint8_t actual_branch_taken = 0;
	uint32_t pc_next = id_ex.PC_next;
	uint8_t jump_taken = 0;
	
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
			log_debug("branch compare");
			//branch compare
			//result = temp
			//pc_next = arithmetic... pc + imm
			//why do the above swap ? the forwarding logic expects value to be written in register to be in result field
			//branch compare and set the branch taken signal's SECOND LSB
			//
			result = id_ex.PC_next;
			if(control.branch_opcode == 0b100){
				log_debug("jump instruction");
				//jump
				pc_next = arithmetic_unit(operand1, operand2, control.arithmetic_opcode);
				//set the branch taken bit too
				jump_taken = 1;
			}else{
				//branch
				log_debug("branch instruction");
				pc_next = arithmetic_unit(id_ex.PC, id_ex.imm, control.arithmetic_opcode);
				actual_branch_taken = branch_comparator(operand1, operand2, control.branch_opcode, control.sign);
				//store
				store(actual_branch_taken);
			}
			break;
		default:
			log_fatal("invalid execute stage");
			exit(1);
	}

	//resolve the store data before ex_ma is overwritten, only stores carry data in rs2
	uint32_t store_data = id_ex.R2;
	if(id_ex.cs_ma.mem == 1 && (id_ex.cs_ma.read_write_opcode & 0x8) == 0x8){
		log_info("Checking for data hazards in R2 field (ex_forward)");
		store_data = forward_ex(id_ex.R2, id_ex.nrs2);
	}

	//fill up the ex_ma register
	
	ex_ma.result = result;
	ex_ma.cs_ma = id_ex.cs_ma;
	//after copying the original cs_ma, add the actual branch taken bit...
	ex_ma.cs_ma.branch_taken |= ((actual_branch_taken | jump_taken) << 1);	//doing this even if it is not a branch
								//if control hasnt been in the above branch part of the code(else part)
								//then the value of actual_branch_taken will be zero, cuz its been initialized to 0
								//and oring with 0 doesnt cause a problem...
	ex_ma.cs_wb = id_ex.cs_wb;
	ex_ma.nrs2 = id_ex.nrs2;

	ex_ma.R2 = store_data;

	ex_ma.nrd = id_ex.nrd;
	ex_ma.PC_next = pc_next;
	ex_ma.ins = id_ex.ins;




}
