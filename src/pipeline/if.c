#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"

//this doesnt handle a memory stall MUST check mfc_i immediately after calling this function !!!!!!!!!!!

void if_stage(){
	log_info("IF stage initiating.");
	if(mfc_i == COMPLETED){			//if memory i port is free, load pc into mar
		mar_i = pc;
		log_debug("initialising memory read in if stage");
	}

	read_memory_i(0);		//call whether or not mfc_i is COMPLETED or NOT_COMPLETED	(in case of a stall if_stage wont be called)
	
	if(mfc_i == COMPLETED){
		log_debug("fetch complete.");
		//load ir
		ir = mbr_i;
		//load the if_id pipeline register
		if_id.IR = ir;
		/*
		if(ir == 0){
			log_fatal("Is ir supposed to be zero ?");
			exit(1);
		}
		*/
		if_id.PC = pc;

		//increment pc
		pc += 4;

		//store the incremented pc in pc-temp;
		if_id.PC_next = pc;

		if_id.ins = instructions[(pc-4)/4];

		log_info("IF stage successful");
	}else{
		log_info("IF stage fail. Injecting a bubble.");
		if_id.IR = 0;
		if_id.PC = 0;
		if_id.PC_next = 0;
		if_id.ins = "                    ";
	}
}

