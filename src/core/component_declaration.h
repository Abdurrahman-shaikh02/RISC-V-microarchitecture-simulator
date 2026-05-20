uint32_t reg_file[32];

uint32_t pc;

uint32_t ir;

int PAUSE;
int TRAP;
int FORWARDING_STALL;
int HAZARD;

IF_ID if_id;
ID_EX id_ex;
EX_MA ex_ma;
MA_WB ma_wb;
WB_IF wb_if;
WB_IF recent_wb;

int FORWARDING_SWITCH = 1;	//forwarding on or off ?
