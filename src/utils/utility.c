#include "header.h"
#include "memory.h"
#include "internal_memory.h"
#include "utility.h"
#include "control.h"
#include "pipeline.h"



void display_general_purpose_registers(char * path){
	FILE * fd = NULL;

	if(path == NULL){
		fd = stdout;
	}else{
		fd = fopen(path, "w");
	}


	if(fd == NULL){
		log_fatal("Couldnt open file for register dump.");
		exit(1);
	}

	for(int i = 0; i < 32; i++){
		fprintf(fd, "r%02d : %08x\n", i, reg_file[i]);
	}

	if(path != NULL){
		fclose(fd);
	}
}

void display_internal_registers(char * path) {
	FILE * fd = NULL;

	if(path == NULL){
		fd = stdout;
	}else{
		fd = fopen(path, "w");
	}


	if(fd == NULL){
		log_fatal("Couldnt open file for internal register dump.");
		exit(1);
	}


	fprintf(fd, "========================================\n");
	fprintf(fd, "CORE REGISTERS\n");
	fprintf(fd, "========================================\n");

	fprintf(fd, "pc       : %08x\n", pc);
	fprintf(fd, "ir       : %08x\n", ir);

	fprintf(fd, "\n");

	fprintf(fd, "========================================\n");
	fprintf(fd, "MEMORY INTERFACE\n");
	fprintf(fd, "========================================\n");

	fprintf(fd, "mar      : %08x\n", mar);
	fprintf(fd, "mbr      : %08x\n", mbr);
	fprintf(fd, "mar_i    : %d\n", mar_i);
	fprintf(fd, "mbr_i    : %d\n", mbr_i);

	fprintf(fd, "\n");

	fprintf(fd, "========================================\n");
	fprintf(fd, "PIPELINE REGISTERS\n");
	fprintf(fd, "========================================\n");

	// ---------------- IF/ID ----------------

	fprintf(fd, "IF_ID:\n");
	fprintf(fd, "IR       : %08x\n", if_id.IR);
	fprintf(fd, "PC       : %08x\n", if_id.PC);
	fprintf(fd, "PC_next  : %08x\n", if_id.PC_next);

	fprintf(fd, "----------------------------------------\n");

	// ---------------- ID/EX ----------------

	fprintf(fd, "ID_EX:\n");
	fprintf(fd, "PC       : %08x\n", id_ex.PC);
	fprintf(fd, "PC_next  : %08x\n", id_ex.PC_next);
	fprintf(fd, "R1       : %08x\n", id_ex.R1);
	fprintf(fd, "R2       : %08x\n", id_ex.R2);
	fprintf(fd, "imm      : %08x\n", id_ex.imm);
	fprintf(fd, "nrd      : %08x\n", id_ex.nrd);

	fprintf(fd, "\n");

	fprintf(fd, "EX CONTROL:\n");
	fprintf(fd, "source1             : %02x\n", id_ex.cs_ex.source1);
	fprintf(fd, "source2             : %02x\n", id_ex.cs_ex.source2);
	fprintf(fd, "sign                : %02x\n", id_ex.cs_ex.sign);
	fprintf(fd, "type                : %02x\n", id_ex.cs_ex.type);
	fprintf(fd, "arithmetic_opcode   : %02x\n", id_ex.cs_ex.arithmetic_opcode);
	fprintf(fd, "logic_opcode        : %02x\n", id_ex.cs_ex.logic_opcode);
	fprintf(fd, "branch_opcode       : %02x\n", id_ex.cs_ex.branch_opcode);
	fprintf(fd, "SYSTEM_CALL         : %02x\n", id_ex.cs_ex.SYSTEM_CALL);

	fprintf(fd, "\n");

	fprintf(fd, "MA CONTROL:\n");
	fprintf(fd, "mem                 : %02x\n", id_ex.cs_ma.mem);
	fprintf(fd, "read_write_opcode   : %02x\n", id_ex.cs_ma.read_write_opcode);
	fprintf(fd, "branch_taken        : %02x\n", id_ex.cs_ma.branch_taken);
	fprintf(fd, "jump_or_branch      : %02x\n", id_ex.cs_ma.jump_or_branch);

	fprintf(fd, "\n");

	fprintf(fd, "WB CONTROL:\n");
	fprintf(fd, "wb                  : %02x\n", id_ex.cs_wb.wb);

	fprintf(fd, "----------------------------------------\n");

	// ---------------- EX/MA ----------------

	fprintf(fd, "EX_MA:\n");
	fprintf(fd, "PC_next  : %08x\n", ex_ma.PC_next);
	fprintf(fd, "R2       : %08x\n", ex_ma.R2);
	fprintf(fd, "result   : %08x\n", ex_ma.result);
	fprintf(fd, "nrd      : %08x\n", ex_ma.nrd);

	fprintf(fd, "\n");

	fprintf(fd, "MA CONTROL:\n");
	fprintf(fd, "mem                 : %02x\n", ex_ma.cs_ma.mem);
	fprintf(fd, "read_write_opcode   : %02x\n", ex_ma.cs_ma.read_write_opcode);
	fprintf(fd, "branch_taken        : %02x\n", ex_ma.cs_ma.branch_taken);
	fprintf(fd, "jump_or_branch      : %02x\n", ex_ma.cs_ma.jump_or_branch);

	fprintf(fd, "\n");

	fprintf(fd, "WB CONTROL:\n");
	fprintf(fd, "wb                  : %02x\n", ex_ma.cs_wb.wb);

	fprintf(fd, "----------------------------------------\n");

	// ---------------- MA/WB ----------------

	fprintf(fd, "MA_WB:\n");
	fprintf(fd, "result   : %08x\n", ma_wb.result);
	fprintf(fd, "nrd      : %08x\n", ma_wb.nrd);

	fprintf(fd, "\n");

	fprintf(fd, "WB CONTROL:\n");
	fprintf(fd, "wb                  : %02x\n", ma_wb.cs_wb.wb);

	fprintf(fd, "========================================\n");

	if(path != NULL){
		fclose(fd);
	}
}

