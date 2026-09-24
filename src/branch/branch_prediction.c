#include "header.h"
#include "branch_prediction.h"
#include "static_branch_prediction.h"
#include "dynamic_branch_prediction.h"


branch_prediction_strategy BRANCH_PREDICTION_STRATEGY = 0;

uint8_t predict(uint32_t ir){
	switch(BRANCH_PREDICTION_STRATEGY){
		case NOT_TAKEN:
			return predict_not_taken(ir);
		case TAKEN:
			return predict_taken(ir);
		case OFFSET:
			return predict_based_on_offset(ir);
		case TWO_BIT_SWITCH:
			return taken_not_taken_switch(ir);
		case HISTORY_TABLE:
			return branch_history_table(ir);
		default:
			log_fatal("Invalid branch prediction strategy.");
			exit(1);
	}
}

void store(uint8_t branch_taken){
	switch(BRANCH_PREDICTION_STRATEGY){
		case TWO_BIT_SWITCH:
			store_taken_not_taken_switch(branch_taken);
		case HISTORY_TABLE:
			store_branch_history_table(branch_taken);
		default:
			// the other predictors do not need the store call
			return;
	}

}
