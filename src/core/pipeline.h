//below are the pipeline registers

typedef struct IF_ID{
	uint32_t IR;
	uint32_t PC;
	uint32_t PC_next;	//stores PC + 4
} IF_ID;
extern IF_ID if_id;

typedef struct ID_EX{
	uint32_t PC;
	uint32_t PC_next;
	uint32_t R1;
	uint32_t R2;
	uint32_t imm;
	uint32_t nrd;		//destination register number
	ex_control cs_ex;	//control signals
	ma_control cs_ma;	//control signals
	wb_control cs_wb;	//control signals
}ID_EX;
extern ID_EX id_ex;

typedef struct EX_MA{
	uint32_t PC_next;
	uint32_t R2;
	uint32_t result;
	uint32_t nrd;		//destination register number
	ma_control cs_ma;
	wb_control cs_wb;
} EX_MA;
extern EX_MA ex_ma;

typedef struct MA_WB{
	uint32_t result;
	uint32_t nrd;		//destination register number
	wb_control cs_wb;
} MA_WB;
extern MA_WB ma_wb;



void if_stage();
void id_stage();
void ex_stage();
void ma_stage();
void wb_stage();


