//below are the grouped control signals

typedef struct ex_control{
	uint8_t source1;	//pc or rs1?		0, 1
	uint8_t source2;	//rs2 or imm?		0, 1
	uint8_t sign;		//unsigned op(0) or signed op(1) ?	0, 1
	uint8_t type;		//arithmetic | logic | less than(slt) | branch compare? 00, 01, 10, 11
	uint8_t arithmetic_opcode;		//alu opcode 0, 1
	uint8_t logic_opcode;		//logic opcode	000 ... 110
	uint8_t branch_opcode;		//branch comparison opcode 000...
	uint8_t SYSTEM_CALL;		//for ebreak, a SYSTEMCALL
} ex_control;

typedef struct ma_control{
	uint8_t mem;		//memory access or no ?
	uint8_t read_write_opcode;	//read or write ? memory access code.... 0000...0100 for reads, 1000...1010 for writes (b, h, w, bu, hu) the 4th last bit gives whether its a read or a write...
	uint8_t branch_taken;	//was the branch taken ?
	uint8_t jump_or_branch;	//was this a branch (no address needs to be writtenback) or a jump (save return address) ? 0,1
} ma_control;

typedef struct wb_control{
	uint8_t wb;		//is this a writeback or idle ?
} wb_control;


typedef struct i_block {
	const char * operation;			//why not make a operation[32] --> cuz ur not going to edit it... its unnecessary to do so.
	int number_of_operands;
	char op1;
	char op2;
	char op3;
	char format;
	int opcode;
	int funct3;
	int funct7;
} i_block;


typedef struct cu_output{
	char format;		//just a guide for the immediate generator...
	ex_control cs_ex;
	ma_control cs_ma;
	wb_control cs_wb;
} cu_output;

cu_output read_r_format(uint32_t ir);
cu_output read_i_format(uint32_t ir);
cu_output read_s_format(uint32_t ir);
cu_output read_b_format(uint32_t ir);
cu_output read_u_format(uint32_t ir);
cu_output read_j_format(uint32_t ir);

//const i_block * find_block(uint32_t opcode, int32_t funct3, int32_t funct7);	//give -1 if you dont expect that field
char get_format(uint32_t opcode);
cu_output decode(uint32_t ir);

extern int PAUSE;	//for ebreak
extern int TRAP;	//for ecall
extern int FORWARDING_STALL;	//for a compulsory hazard stall
extern int HAZARD;		//set when a hazard is detected if forwarding is off main can decide to force a stall and clear the reg...
