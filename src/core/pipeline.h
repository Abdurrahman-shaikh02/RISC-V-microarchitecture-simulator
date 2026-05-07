typedef struct IF_ID{
	int IR;
	int PC;
} IF_ID;
static IF_ID if_id;

typedef struct ID_EX{
	int R1;
	int R2;
	int imm;
	int cs_ex;	//control signals
	int cs_ma;	//control signals
	int cs_wb;	//control signals include the destination register number...
}ID_EX;
static ID_EX id_ex;

typedef struct EX_MA{

} EX_MA;
static EX_MA ex_ma;

typedef struct MA_WB{

} MA_WB;
static MA_WB ma_wb;



void if_stage();
void id_stage();
void ex_stage();
void ma_stage();
void ex_stage();
