#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"
#include "alu.h"
#include "branch_prediction.h"
#include "stats.h"


uint32_t forward_ex(uint32_t original_value, uint32_t n_reg){
	//function checks for hazard wrt to the given register number n_reg...
	//if there is a hazard, it forwards value and returns it
	//if not it returns original_value
	
	if(ex_ma.cs_wb.wb == 1 && (n_reg == ex_ma.nrd && ex_ma.nrd != 0)){
		//first lets check in ex_ma... this moment it contains the result of the ex stage of the previous clock cycle.
		//if we find it here then we need to see if its a memory load operation
		//if yes then we must stall and then wait for the ma stage to complete... then we can have our value
		//if no then just use the result directly no problem with that...
		if(ex_ma.cs_ma.mem == 1){
			//compulsory stall if its a memory read (for a mem write wb = 0 so no question of that)
			log_info("Compulsory stall due to hazard.");
			FORWARDING_STALL = 1;
		}else{
			log_debug("forwarding from ex_ma...");
			n_forwards_to_ex++;		//stat
			return ex_ma.result;
		}
	}else if(ma_wb.cs_wb.wb == 1 && (n_reg == ma_wb.nrd && ma_wb.nrd != 0)){
		//if we find our value here then it MUST NECESSARILY mean that the memory operation just got completed this cycle(ma happened before ex)
		//(if it was something like an addi which has an idle ma stage then we would have caught it in the above clause in the ex_ma register)
		//and therefore we must cause a stall to skip the current cycle
		log_info("Must give a stall because the memory action got completed in the current cycle... cant forward it...");
		FORWARDING_STALL = 1;
	}else if(wb_if.cs_wb.wb == 1 && (n_reg == wb_if.nrd && wb_if.nrd != 0)){
		//if we find our value here then we can directly use the value...
		//it could either be after a memory access or maybe we had an instruction b/w the hazard causing instruction and the current instruction
		log_debug("forwarding from ma_wb...");
		n_forwards_to_ex++;		//stat
		return wb_if.result;
	}else{
		log_info("No forwarding required...");
		return original_value;
	}

	return 0;
}


uint32_t forward_ma(uint32_t original_value, uint32_t n_reg){
	//function checks for hazard wrt to the given register number n_reg...
	//if there is a hazard, it forwards value and returns it
	//if not it returns original_value

	if(ma_wb.cs_wb.wb == 1 && (ma_wb.nrd == n_reg && ma_wb.nrd != 0)){
		//why only check ma_wb not the value of wb_if ?
		//because the wb stage of the current cycle has already happened and the value of ma_wb is the same as wb_if...
		log_info("forwarding from ma_wb...");
		n_forwards_to_ma++;		//stat
		return ma_wb.result;
	}else if(recent_wb.cs_wb.wb == 1 && (recent_wb.nrd == n_reg && recent_wb.nrd != 0)){
		//ideally i should be saving the wb before the recent wb too...
		//that would be the write back stage during the decode of this instruction...
		//but since our wb happens before decode does we (luckily!!) got that value right during decode....
		log_info("forwarding from recent_wb...");
		n_forwards_to_ma++;		//stat
		return recent_wb.result;
	}else{
		log_info("No forwarding required...");
		return original_value;
	}
}
