#include "header.h"
#include "branch_prediction.h"


uint8_t predict(uint32_t ir){
	switch(BRANCH_PREDICTION_STRATEGY){
		case 0:
			return predict_not_taken(ir);
		case 1:
			return predict_taken(ir);
		case 2:
			return predict_based_on_offset(ir);
		case 3:
			return taken_not_taken_switch(ir);
		case 4:
			return branch_history_table(ir);
		default:
			log_fatal("Invalid branch prediction strategy.");
			exit(1);
	}
}

void store(uint8_t branch_taken){

}
