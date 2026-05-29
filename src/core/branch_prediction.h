extern int BRANCH_PREDICTION_STRATEGY;

int predict_not_taken(uint32_t ir);
int predict_taken(uint32_t ir);
int predict_based_on_offset(uint32_t ir);
int taken_not_taken_switch(uint32_t ir);
int branch_history_table(uint32_t ir);

uint8_t predict(uint32_t ir);
void store(uint8_t);
