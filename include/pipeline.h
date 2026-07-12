extern uint32_t reg_file[];

//below are the pipeline registers

typedef struct IF_ID{
	uint32_t IR;
	uint32_t PC;
	uint32_t PC_next;	//stores PC + 4
	char * ins;
} IF_ID;
extern IF_ID if_id;

typedef struct ID_EX{
	uint32_t PC;
	uint32_t PC_next;
	uint32_t nrs1;
	uint32_t R1;
	uint32_t nrs2;
	uint32_t R2;
	uint32_t imm;
	uint32_t nrd;		//destination register number
	ex_control cs_ex;	//control signals
	ma_control cs_ma;	//control signals
	wb_control cs_wb;	//control signals
	char * ins;
}ID_EX;
extern ID_EX id_ex;

typedef struct EX_MA{
	uint32_t PC_next;
	uint32_t nrs2;
	uint32_t R2;		//this is because the data in case of a write is encoded as rs2
	uint32_t result;
	uint32_t nrd;		//destination register number
	ma_control cs_ma;
	wb_control cs_wb;
	char * ins;
} EX_MA;
extern EX_MA ex_ma;

typedef struct MA_WB{
	uint32_t result;
	uint32_t nrd;		//destination register number
	wb_control cs_wb;
	char * ins;
} MA_WB;
extern MA_WB ma_wb;

typedef struct WB_IF{
	uint32_t result;
	uint32_t nrd;		//destination register number
	wb_control cs_wb;
	//copied the above three things to let the forwarding logic know that a hazard has been detected in the MA stage...
	//why this ?
	//due to our design where wb and ma stage of the current cycle happen before the ex stage... the data in MA_WB that should be used to
	//detect hazards from MA stage are getting moved ahead before getting detected by the exec stage...
	char * ins;
} WB_IF;
extern WB_IF wb_if;
extern WB_IF recent_wb;		//solely for the purpose of saving a history of the write backs... only for the forwarding to the ma stage.

void if_stage();
void id_stage();
void update_operands();
void reset_decode_flags();
void ex_stage();
void ma_stage();
void wb_stage();


