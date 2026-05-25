#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"


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
		}else{
			//branch
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
			//forwarding logic
			if(ma_wb.cs_wb.wb == 1 && (ma_wb.nrd == ex_ma.nrs2 && ma_wb.nrd != 0)){
				//why only check ma_wb not the value of wb_if ?
				//because the wb stage of the current cycle has already happened and the value of ma_wb is the same as wb_if...
				log_info("forwarding value of rs2 from ex_ma in ma stage");
				mbr = ma_wb.result;
			}else if(recent_wb.cs_wb.wb == 1 && (recent_wb.nrd == ex_ma.nrs2 && recent_wb.nrd != 0)){
				//ideally i should be saving the wb before the recent wb too...
				//that would be the write back stage during the decode of this instruction...
				//but since our wb happens before decode does we (coincidentaly... luckily) got that value right during decode....
				log_info("forwarding value of rs2 from recent wb in ma stage");
				mbr = recent_wb.result;
			}else{
				mbr = ex_ma.R2;
				log_info("no forwarding done...");
			}

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
