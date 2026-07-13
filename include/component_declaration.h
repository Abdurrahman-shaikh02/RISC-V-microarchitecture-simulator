uint64_t cycle;		//the global cycle count

uint32_t reg_file[32];

uint32_t pc;

uint32_t ir;

int PAUSE;
int TRAP;
int FORWARDING_STALL;	//used for the compulsory stall during forwarding to ex stage
int HAZARD_STALL;	//set when a stall is required... only used when forwarding switch is on
int STRUCTURAL_HAZARD_STALL;
int FLUSH;

IF_ID if_id = (IF_ID){0, 0, 0, "                    "};
ID_EX id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};
EX_MA ex_ma = (EX_MA){0, 0, 0, 0, 0, {0, 0, 0, 0}, {0}, "                    "};
MA_WB ma_wb = (MA_WB){0, 0, {0}, "                    "};
WB_IF wb_if = (WB_IF){0, 0, {0}, "                    "};
WB_IF recent_wb = (WB_IF){0, 0, {0}, "                    "};

int FORWARDING_SWITCH = 1;	//forwarding on(1) or off(0) ?

//replace the following with enums
int BRANCH_PREDICTION_STRATEGY = 0;	//0 not taken
					//1 taken
					//2 based on offset
					//3 taken not taken switch
					//4 branch history table

