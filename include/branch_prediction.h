typedef enum branch_prediction_strategy {
	NOT_TAKEN,
	TAKEN,
	OFFSET,
	TWO_BIT_SWITCH,
	HISTORY_TABLE
} branch_prediction_strategy;

extern branch_prediction_strategy BRANCH_PREDICTION_STRATEGY;

uint8_t predict(uint32_t ir);

void store(uint8_t);
