extern int BRANCH_PREDICTION_STRATEGY;

int predict_not_taken(uint32_t ir);
int predict_taken(uint32_t ir);
int predict_based_on_offset(uint32_t ir);
int taken_not_taken_switch(uint32_t ir);
int branch_history_table(uint32_t ir);
uint8_t predict(uint32_t ir);


void store_taken_not_taken_switch(uint8_t branch_taken);
void store_branch_history_table(uint8_t branch_taken);
void store(uint8_t);
