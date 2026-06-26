#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"
#include "alu.h"

int set_hazard_stall();


void id_stage(){
	//the following segment is only due to our design where id happens before if does...
	//so if we change the value of pc here it would be the same as if the pc value was changed in the last clock cycle...
	static int change_pc = 0;
	static uint32_t jump_pc = 0;
	if(change_pc == 1){
		//change pc, reset id_ex reg and if_id
		//now this is sort of a trigger set in the last clock cycle to change the pc because the branch prediction predicted
		//in the last clock cycle that the branch will be taken 
		pc = jump_pc;
		FLUSH = 1;
		
		jump_pc = 0;
		change_pc = 0;
		log_debug("made a branch taken prediction last clock cycle...");
		return;
	}
	//-------------------------------------------------------------------------------------------------------------------------

	log_info("ID stage initiating.");
	//read contents from if_id
	uint32_t ir = if_id.IR;
	//uint32_t pc = if_id.PC;
	//break the opcode into source1, source2, immediate, control signals(maybe add a seperate field for dest reg or include in here)
	
	//get the registers first...(no decoding required)
	uint32_t nrd = (ir & 0x00000F80) >> 7;
	uint32_t nrs1 = (ir & 0x000F8000) >> 15;
	uint32_t nrs2 = (ir & 0x01F00000) >> 20;
	
	cu_output out = decode(if_id.IR);
	log_info("Decode complete.");

	// generate immediate as well generate_immediate dispatches to function like read_i/../j_immediate and just give the immediate value(do not left shift the address wala encoded immediate)...!
	uint32_t imm = generate_immediate(out.format, if_id.IR);
	log_info("Immediate generation complete.");

	//copy everything into id_ex
	id_ex.PC = if_id.PC;
	id_ex.PC_next = if_id.PC_next;
	id_ex.nrs1 = nrs1;
	id_ex.R1 = reg_file[nrs1];
	id_ex.nrs2 = nrs2;
	id_ex.R2 = reg_file[nrs2];
	id_ex.imm = imm;
	id_ex.nrd = nrd;
	id_ex.cs_ex = out.cs_ex;
	id_ex.cs_ma = out.cs_ma;
	id_ex.cs_wb = out.cs_wb;
	id_ex.ins = if_id.ins;

	
	
	if(FORWARDING_SWITCH == 0){
		if(set_hazard_stall() == 1){
			return;		//return only if a stall was set
		}
	}
	
	//after hazard stall so if u are stallin gthen this trigger wont set
	if(out.cs_ma.branch_taken == 1 && out.cs_ex.type == 0b11 && out.cs_ma.jump_or_branch == 0){
		//if it s a branch insturctions and the prediction value is 1
		//set change_pc = 1 to be read in the next cycle...
		change_pc = 1;
		jump_pc = if_id.PC + generate_immediate('b', ir);
	}
}

int set_hazard_stall(){
	//returns 1 if stall was set
	//this will analyse the current machine state and set a stall if required...
	//CATCH THE STALL IN MAIN
	//first check ex_ma
	//then ma_wb
	//then wb_if --> has been written in the current cycle must stall
	

	if(ex_ma.cs_wb.wb == 1){
		if((id_ex.cs_ex.source1 == 1 && ex_ma.nrd == id_ex.nrs1 && id_ex.nrs1 != 0) || (id_ex.cs_ex.source2 == 0 && ex_ma.nrd == id_ex.nrs2 && id_ex.nrs2 != 0)){
			//if either of the current operands has a pending write in the ex_ma
			HAZARD_STALL = 1;
			return 1;
		}
	}else if(ma_wb.cs_wb.wb == 1){
		if((id_ex.cs_ex.source1 == 1 && ma_wb.nrd == id_ex.nrs1 && id_ex.nrs1 != 0) || (id_ex.cs_ex.source2 == 0 && ma_wb.nrd == id_ex.nrs2 && id_ex.nrs2 != 0)){
			//if either of the current operands has a pending write in the ma_wb
			HAZARD_STALL = 1;
			return 1;
		}
	}else if(wb_if.cs_wb.wb == 1){
		if((id_ex.cs_ex.source1 == 1 && wb_if.nrd == id_ex.nrs1 && id_ex.nrs1 != 0) || (id_ex.cs_ex.source2 == 0 && wb_if.nrd == id_ex.nrs2 && id_ex.nrs2 != 0)){
			//if either of the current operands has been written to in the current cycle in wb stage
			//still need to stall because due to our design wb stage happens before id in the same clock
			HAZARD_STALL = 1;
			return 1;
		}
	}

	return 0;
}

void update_operands(){
	id_ex.R1 = reg_file[id_ex.nrs1];
	id_ex.R2 = reg_file[id_ex.nrs2];
	log_info("Updating operands in decode stage due to stall");
}
