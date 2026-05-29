#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"
#include "alu.h"
#include "branch_prediction.h"


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
			//here need to check for hazard first
			//see in EX_MA if it is (set HAZARD = 1) 
			//a write back and not a mem load instruction... if yes then copy result from EX_MA
			//if it is a wb and a mem load as well then its a compulory stall and hence HAZARD_STALL = 1(handle this in main)
			//if its not there in EX_MA then look into MA_WB
			//if yes (HAZARD = 1) then copy directly (because if it is in MA_WB that would mean that the memory load has finished)
			//-------------------------------------------------------------------------------------------------------------
			if((ex_ma.cs_wb.wb == 1 && (id_ex.nrs1 == ex_ma.nrd && ex_ma.nrd != 0)) || (ma_wb.cs_wb.wb == 1 && (id_ex.nrs1 == ex_ma.nrd && ex_ma.nrd != 0))){
				//in the next cycle since ma happens before ex, the value maybe there...
				//but that cycle must be skipped... because the value was just fetched that clock cycle and cant be used in ex...
				log_info("hazard detected rs1");
				if(ex_ma.cs_ma.mem == 1){
					//compulsory stall if its a memory read (for a mem write wb = 0 so no question of that)
					log_info("Compulsory stall due to hazard.");
					FORWARDING_STALL = 1;
				}else{
					log_debug("forwarding rs1 from ex_ma...");
					operand1 = ex_ma.result;
				}
			}else if(wb_if.cs_wb.wb == 1 && (id_ex.nrs1 == wb_if.nrd && wb_if.nrd != 0)){
				//why check wb_if and not ma_wb
				//because due to our design wb and ma of the current cycle are getting executed before ex
				//this is causing the value to move up ahead of ma_wb
				//forward directly
				log_debug("forwarding rs1 from ma_wb...");
				operand1 = wb_if.result;
			}else{
				operand1 = id_ex.R1;
			}
			//-------------------------------------------------------------------------------------------------------------
			log_debug("r1 = rs1");
			break;
		default:
			log_fatal("Invalid source 1.");
			exit(1);
	}

	uint32_t operand2 = 0;
	if(control.source2 == 0){
		//check for hazard first...
		//same as above
		if((ex_ma.cs_wb.wb == 1 && (id_ex.nrs2 == ex_ma.nrd && ex_ma.nrd != 0)) || (ma_wb.cs_wb.wb == 1 && (id_ex.nrs2 == ex_ma.nrd && ex_ma.nrd != 0))){
			//in the next cycle since ma happens before ex, the value maybe there...
			//but that cycle must be skipped... because the value was just fetched that clock cycle and cant be used in ex...
			log_info("hazard detected rs2");
			if(ex_ma.cs_ma.mem == 1){
				//compulsory stall if its a memory read (for a mem write wb = 0 so no question of that)
				log_info("Compulsory stall due to hazard.");
				FORWARDING_STALL = 1;
			}else{
				log_debug("forwarding rs2 from ex_ma...");
				operand2 = ex_ma.result;
			}
		}else if(wb_if.cs_wb.wb == 1 && (id_ex.nrs2 == wb_if.nrd && wb_if.nrd != 0)){
			//why check wb_if and not ma_wb
			//because due to our design wb and ma of the current cycle are getting executed before ex
			//this is causing the value to move up ahead of ma_wb
			//forward directly
			log_debug("forwarding rs2 from ma_wb...");
			operand2 = wb_if.result;
		}else{
			operand2 = id_ex.R2;
		}
		log_debug("r2 = rs2");
	}else if(control.source2 == 1){
		operand2 = id_ex.imm;
		log_debug("r2 = imm");
	}

	uint32_t result = 0;
	uint8_t actual_branch_taken = 0;
	
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
				id_ex.PC_next = arithmetic_unit(operand1, operand2, control.arithmetic_opcode);
				//set the branch taken bit too
				id_ex.cs_ma.branch_taken |= 0b10;
			}else{
				//branch
				log_debug("branch instruction");
				id_ex.PC_next = arithmetic_unit(id_ex.PC, id_ex.imm, control.arithmetic_opcode);
				printf("%08x, %08x\n", operand1, operand2);
				actual_branch_taken = branch_comparator(operand1, operand2, control.branch_opcode, control.sign);
				//store
				store(actual_branch_taken);
			}
			break;
		default:
			log_fatal("invalid execute stage");
			exit(1);
	}

	//fill up the ex_ma register
	
	ex_ma.result = result;
	ex_ma.cs_ma = id_ex.cs_ma;
	//after copying the original cs_ma, add the actual branch taken bit...
	ex_ma.cs_ma.branch_taken |= (actual_branch_taken << 1);	//doing this even if it is not a branch
								//if control hasnt been in the above branch part of the code(else part)
								//then the value of actual_branch_taken will be zero, cuz its been initialized to 0
								//and oring with 0 doesnt cause a problem...
	ex_ma.cs_wb = id_ex.cs_wb;
	ex_ma.nrs2 = id_ex.nrs2;
	ex_ma.R2 = id_ex.R2;
	ex_ma.nrd = id_ex.nrd;
	ex_ma.PC_next = id_ex.PC_next;
	ex_ma.ins = id_ex.ins;




}
