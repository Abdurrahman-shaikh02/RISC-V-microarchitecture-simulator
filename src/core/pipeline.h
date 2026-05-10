#include <stdint.h>

typedef struct IF_ID{
	uint32_t IR;
	uint32_t PC;
} IF_ID;
static IF_ID if_id;

typedef struct ID_EX{
	uint32_t R1;
	uint32_t R2;
	uint32_t imm;
	uint32_t cs_ex;	//control signals
	uint32_t cs_ma;	//control signals
	uint32_t cs_wb;	//control signals include the destination register number...
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

typedef enum stage{
	IF,
	ID,
	EX,
	MA,
	WB
} stage;

void stall_pipeline(stage type);
