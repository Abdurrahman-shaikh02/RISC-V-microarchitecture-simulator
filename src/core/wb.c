#include "header.h"
#include "memory.h"
#include "internal_memory.h"
#include "control.h"
#include "pipeline.h"


void wb_stage(){
	log_info("WB stage initiating.");

	wb_control control = ma_wb.cs_wb;
	if(control.wb == 0){
		log_info("No write back.");
	}else if(control.wb == 1){
		log_info("Writing back.");
		reg_file[ma_wb.nrd] = ma_wb.result;

		reg_file[0] = 0;		//ensure after every write that r0 is 0
	}
}
