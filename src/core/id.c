#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"

//NEED TO HANDLE SIGN EXTENSION
uint32_t generate_immediate(char format){
	uint32_t ir = if_id.IR;
	uint32_t imm = 0;

	switch(format){
		case 0:
			//its a bubble
			log_debug("no immediate was generated for a bubble");
			return 0;
		case 'r':
			log_debug("no immediate was generated");
			return 0;
		case 'i':
			imm = (int32_t)(ir & 0xFFF00000) >> 20;
			log_debug("immediate generated for i format");
			break;
		case 's':
			imm = ir & 0xFE000000;
			imm = (ir & 0x00000F80) << 13;
			imm = ((int32_t)imm) >> 20;
			log_debug("immediate generated for s format");
			break;
		case 'b':
		case 'u':
			imm = (int32_t)(ir & 0xFFFFF000) >> 12;
			log_debug("immediate generated for u format");
			break;
		case 'j':
		default:
			log_fatal("invalid format for immediate generation");
			exit(1);
	}

	return imm;
}

void id_stage(){
	log_info("ID stage initiating.");
	//read contents from if_id
	uint32_t ir = if_id.IR;
	uint32_t pc = if_id.PC;
	//break the opcode into source1, source2, immediate, control signals(maybe add a seperate field for dest reg or include in here)
	
	//get the registers first...(no decoding required)
	uint32_t nrd = (ir & 0x00000F80) >> 7;
	uint32_t nrs1 = (ir & 0x000F8000) >> 15;
	uint32_t nrs2 = (ir & 0x01F00000) >> 20;
	
	cu_output out = decode(ir);
	log_info("Decode complete.");

	// generate immediate as well generate_immediate dispatches to function like read_i/../j_immediate and just give the immediate value(do not left shift the address wala encoded immediate)...!
	uint32_t imm = generate_immediate(out.format);
	log_info("Immediate generation complete.");

	//copy everything into id_ex
	id_ex.PC = if_id.PC;
	id_ex.PC_next = if_id.PC_next;
	id_ex.R1 = reg_file[nrs1];
	id_ex.R2 = reg_file[nrs2];
	id_ex.imm = imm;
	id_ex.nrd = nrd;
	id_ex.cs_ex = out.cs_ex;
	id_ex.cs_ma = out.cs_ma;
	id_ex.cs_wb = out.cs_wb;
}
