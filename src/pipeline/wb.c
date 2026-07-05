#include "header.h"
#include "memory.h"
#include "internal_memory.h"
#include "control.h"
#include "pipeline.h"
#include "stats.h"


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

	//saving history for the forwarding to ma
	recent_wb.ins = wb_if.ins;
	recent_wb.result = wb_if.result;
	recent_wb.nrd = wb_if.nrd;
	recent_wb.cs_wb = wb_if.cs_wb;
	

	wb_if.ins = ma_wb.ins;
	wb_if.result = ma_wb.result;
	wb_if.nrd = ma_wb.nrd;
	wb_if.cs_wb = ma_wb.cs_wb;

	//this is desperate... unable find another way of counting no of instructions... shouldn't use the instruction string - its only for visuals
	if(wb_if.ins != NULL && strcmp(wb_if.ins, "                    ") != 0){
		n_instructions++;	//stat
	}
}
