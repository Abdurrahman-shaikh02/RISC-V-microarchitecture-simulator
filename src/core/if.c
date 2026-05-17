#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"

//this doesnt handle a memory stall MUST check mfc_i immediately after calling this function !!!!!!!!!!!

void if_stage(){
	log_info("IF stage initiating.");
	if(mfc_i == 1){			//if memory i port is free, load pc into mar
		mar_i = pc;
		log_debug("initialising memory read in if stage");
	}

	read_memory_i();		//call whether or not mfc_i is 0 or 1	(in case of a stall if_stage wont be called)
	
	if(mfc_i == 1){
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

		log_info("IF stage successful");
	}else{
		log_info("IF stage fail. Injecting a bubble.");
		if_id.IR = 0;
		if_id.PC = 0;
		if_id.PC_next = 0;
	}
}
	/*
	//attempt to deal with structural hazard between EX and IF stage.. ended up making a second port in memory for instructions.. read only segment
	//access memory location pc
	static uint8_t mfc_initiated = 0;		//has the memory function (read) been initiated for the current pc ?

	if(mfc == 0 && mfc_initiated == 1){	//when read function has been started and mfc isnt 1, then controller is currently serving the if...
		read_memory();			//to increment the internal memory controller counter
		return;
	}

	if(mfc == 1 && mfc_initiated == 0){	//if memory controller is free and the current pc's memory read hasnt been initiated...
		mar = pc;			//then initiate the read function...
		read_memory();
		mfc_initiated = 1;
	}

	if(mfc == 1 && mfc_initiated == 1){	//when the memory function has completed and the rad munction was initiated...
		//load ir
		ir = mbr;
		//load the if_id pipeline register
		if_id.IR = ir;
		if_id.PC = pc;

		//increment pc
		pc += 4;
		mfc_initiated = 0;
	}
	//stall if required...
	*/


