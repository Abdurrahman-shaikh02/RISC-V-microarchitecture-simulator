#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"
#include "stats.h"
#include "forwarding.h"


//this function does not handle memory stalls and the user must check the status of mfc after to decide what to do
//if its a fail the result register must be cleared manually

void ma_stage(){
	log_info("MA stage initiating.");

	ma_control control = ex_ma.cs_ma;
	uint32_t result = ex_ma.result;

	uint8_t r_w = (control.read_write_opcode & 0x8) >> 3;
	uint32_t mem_opcode = (control.read_write_opcode & 0x7);
	
	if(control.mem == 0){
		log_info("No memory access.");
		//if its a jump, result = result
		//and update pc right now... pc = pc_next
		//
		//else if its a branch
		//if the predict and compare bits match then store only...
		//if they dont, store, flush bit
		//if prediction was to take a branch and we didnt take then pc = result
		//if prediction was not to take and we did then update pc = pc_next (remember we swapped values in the last stage...)
		//result = result
		if(control.jump_or_branch == 1){
			pc = ex_ma.PC_next;
			FLUSH = 1;
		}else if(control.jump_or_branch == 2){
			//branch
			n_branch_instructions++;	//stat
			uint8_t predicted = 0, actual = 0;
			actual = (control.branch_taken & 0b10) >> 1;
			predicted = (control.branch_taken & 0b1);

			if(actual == predicted){
				//do nothing
				log_info("branch hit!");
				branch_hit++;		//stat
			}else if(actual == 1 && predicted == 0){
				log_info("branch miss!");
				branch_miss++;		//stat
				pc = ex_ma.PC_next;
				FLUSH = 1;
			}else if(actual == 0 && predicted == 1){
				log_info("branch miss!");
				branch_miss++;		//stat
				pc = ex_ma.result;
				FLUSH = 1;
			}
		}else{
			//its an idle ma stage... (branch/memory = branch but jump/branch = 0)
			//do nothing
		}
		result = ex_ma.result;
	}else{
		//memory access
		log_info("Accessing memory.");
		if(r_w == 0){
			//read
			log_info("Attempting to read.");
			mar = ex_ma.result;
			read_memory(mem_opcode);
			result = mbr;
		}else if(r_w == 1){
			//write
		
			log_info("Checking for data hazards... (ma forward)");
			mbr = forward_ma(ex_ma.R2, ex_ma.nrs2);

			log_info("Attempting to write.");
			mar = ex_ma.result;
			write_memory(mem_opcode);
		}
	}

	//fill up the next register
	ma_wb.result = result;
	ma_wb.cs_wb = ex_ma.cs_wb;
	ma_wb.nrd = ex_ma.nrd;
	ma_wb.ins = ex_ma.ins;

}
