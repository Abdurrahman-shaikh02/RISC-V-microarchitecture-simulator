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
			log_info("Attempting to write.");
			mbr = ex_ma.R2;
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
