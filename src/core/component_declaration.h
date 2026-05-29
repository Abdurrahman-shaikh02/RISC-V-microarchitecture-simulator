uint32_t reg_file[32];

uint32_t pc;

uint32_t ir;

int PAUSE;
int TRAP;
int FORWARDING_STALL;	//used for the compulsory stall during forwarding to ex stage
int HAZARD_STALL;	//set when a stall is required... only used when forwarding switch is on
int FLUSH;

IF_ID if_id;
ID_EX id_ex;
EX_MA ex_ma;
MA_WB ma_wb;
WB_IF wb_if;
WB_IF recent_wb;

int FORWARDING_SWITCH = 1;	//forwarding on(1) or off(0) ?
int BRANCH_PREDICTION_STRATEGY = 0;	//0 not taken
					//1 taken
					//2 based on offset
					//3 taken not taken switch
					//4 branch history table
