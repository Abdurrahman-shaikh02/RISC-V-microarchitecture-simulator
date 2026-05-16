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
	
	if(control.mem == 0){
		log_info("No memory access.");
		//jump or branch
	}else{
		//memory access
		log_info("Accessing memory.");
		if(control.r_w == 0){
			//read
			log_info("Attempting to read.");
			mar = ex_ma.result;
			read_memory();
			result = mbr;
		}else if(control.r_w == 1){
			//write
			log_info("Attempting to write.");
			mbr = ex_ma.R1;
			mar = ex_ma.result;
			write_memory();
		}
	}

	//fill up the next register
	ma_wb.result = result;
	ma_wb.cs_wb = ex_ma.cs_wb;
	ma_wb.nrd = ex_ma.nrd;

}
