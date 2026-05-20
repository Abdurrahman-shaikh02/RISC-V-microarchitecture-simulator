#include "header.h"
#include "memory.h"
#include "internal_memory.h"
#include "utility.h"
#include "control.h"
#include "pipeline.h"



void display_general_purpose_registers(){
	for(int i = 0; i < 32; i++){
		printf("r%02d : %08x\n", i, reg_file[i]);
	}
}

void display_internal_registers() {

	printf("========================================\n");
	printf("CORE REGISTERS\n");
	printf("========================================\n");

	printf("pc       : %08x\n", pc);
	printf("ir       : %08x\n", ir);

	printf("\n");

	printf("========================================\n");
	printf("MEMORY INTERFACE\n");
	printf("========================================\n");

	printf("mar      : %08x\n", mar);
	printf("mbr      : %08x\n", mbr);
	printf("mar_i    : %d\n", mar_i);
	printf("mbr_i    : %d\n", mbr_i);

	printf("\n");

	printf("========================================\n");
	printf("PIPELINE REGISTERS\n");
	printf("========================================\n");

	// ---------------- IF/ID ----------------

	printf("IF_ID:\n");
	printf("IR       : %08x\n", if_id.IR);
	printf("PC       : %08x\n", if_id.PC);
	printf("PC_next  : %08x\n", if_id.PC_next);

	printf("----------------------------------------\n");

	// ---------------- ID/EX ----------------

	printf("ID_EX:\n");
	printf("PC       : %08x\n", id_ex.PC);
	printf("PC_next  : %08x\n", id_ex.PC_next);
	printf("R1       : %08x\n", id_ex.R1);
	printf("R2       : %08x\n", id_ex.R2);
	printf("imm      : %08x\n", id_ex.imm);
	printf("nrd      : %08x\n", id_ex.nrd);

	printf("\n");

	printf("EX CONTROL:\n");
	printf("source1             : %02x\n", id_ex.cs_ex.source1);
	printf("source2             : %02x\n", id_ex.cs_ex.source2);
	printf("sign                : %02x\n", id_ex.cs_ex.sign);
	printf("type                : %02x\n", id_ex.cs_ex.type);
	printf("arithmetic_opcode   : %02x\n", id_ex.cs_ex.arithmetic_opcode);
	printf("logic_opcode        : %02x\n", id_ex.cs_ex.logic_opcode);
	printf("branch_opcode       : %02x\n", id_ex.cs_ex.branch_opcode);
	printf("SYSTEM_CALL         : %02x\n", id_ex.cs_ex.SYSTEM_CALL);

	printf("\n");

	printf("MA CONTROL:\n");
	printf("mem                 : %02x\n", id_ex.cs_ma.mem);
	printf("read_write_opcode   : %02x\n", id_ex.cs_ma.read_write_opcode);
	printf("branch_taken        : %02x\n", id_ex.cs_ma.branch_taken);
	printf("jump_or_branch      : %02x\n", id_ex.cs_ma.jump_or_branch);

	printf("\n");

	printf("WB CONTROL:\n");
	printf("wb                  : %02x\n", id_ex.cs_wb.wb);

	printf("----------------------------------------\n");

	// ---------------- EX/MA ----------------

	printf("EX_MA:\n");
	printf("PC_next  : %08x\n", ex_ma.PC_next);
	printf("R2       : %08x\n", ex_ma.R2);
	printf("result   : %08x\n", ex_ma.result);
	printf("nrd      : %08x\n", ex_ma.nrd);

	printf("\n");

	printf("MA CONTROL:\n");
	printf("mem                 : %02x\n", ex_ma.cs_ma.mem);
	printf("read_write_opcode   : %02x\n", ex_ma.cs_ma.read_write_opcode);
	printf("branch_taken        : %02x\n", ex_ma.cs_ma.branch_taken);
	printf("jump_or_branch      : %02x\n", ex_ma.cs_ma.jump_or_branch);

	printf("\n");

	printf("WB CONTROL:\n");
	printf("wb                  : %02x\n", ex_ma.cs_wb.wb);

	printf("----------------------------------------\n");

	// ---------------- MA/WB ----------------

	printf("MA_WB:\n");
	printf("result   : %08x\n", ma_wb.result);
	printf("nrd      : %08x\n", ma_wb.nrd);

	printf("\n");

	printf("WB CONTROL:\n");
	printf("wb                  : %02x\n", ma_wb.cs_wb.wb);

	printf("========================================\n");
}


/*
void display_internal_registers(){

	printf("pc : %08x\n", pc);
	printf("ir : %08x\n", ir);

	printf("\n\n");

	printf("Memory:\n");
	printf("mar : %08x\n", mar);
	printf("mbr : %08x\n", mbr);
	printf("mar_i : %d\n", mar_i);
	printf("mbr_i : %d\n", mbr_i);
	printf("-------------------------------\n");

	//pipeline registers
	
	printf("IF_ID:\n");
	printf("IR : %08x\n", if_id.IR);
	printf("PC : %08x\n", if_id.PC);
	printf("PC_next : %08x\n", if_id.PC_next);
	printf("-------------------------------\n");


	printf("ID_EX:\n");
	printf("PC : %08x\n", id_ex.PC);
	printf("PC_next : %08x\n", id_ex.PC_next);
	printf("R1 : %08x\n", id_ex.R1);
	printf("R2 : %08x\n", id_ex.R2);
	printf("imm : %08x\n", id_ex.imm);
	printf("nrd : %08x\n", id_ex.nrd);
	printf("R1 : %08x\n", id_ex.R1);

	printf("-------------------------------\n");

}*/
