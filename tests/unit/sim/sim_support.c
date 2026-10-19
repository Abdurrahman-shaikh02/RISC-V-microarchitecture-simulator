#include "sim_support.h"

// the simulator's global machine state is defined in this header (normally by
// src/main.c, which is not linked into the unit tests)
#include "component_declaration.h"

void sim_quiet(void){
	CURRENT_LEVEL = 6;
}

void sim_reset_pipeline(void){
	if_id = (IF_ID){0, 0, 0, BLANK_INS};
	id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, BLANK_INS};
	ex_ma = (EX_MA){0, 0, 0, 0, 0, {0, 0, 0, 0}, {0}, BLANK_INS};
	ma_wb = (MA_WB){0, 0, {0}, BLANK_INS};
	wb_if = (WB_IF){0, 0, {0}, BLANK_INS};
	recent_wb = (WB_IF){0, 0, {0}, BLANK_INS};
	memset(reg_file, 0, sizeof(reg_file));
	pc = 0;
	ir = 0;
	PAUSE = TRAP = FORWARDING_STALL = DATA_HAZARD_STALL = STRUCTURAL_HAZARD_STALL = FLUSH = 0;
	mfc = COMPLETED;
	mfc_i = COMPLETED;
	reset_decode_flags();
}

void sim_setup(int cache_levels){
	sim_quiet();
	configure(NULL);
	CACHE_LEVELS = cache_levels;
	cycle = 0;
	sim_reset_pipeline();
}

void sim_dram_set_word(uint32_t addr, uint32_t w){
	dram[addr + 0] = w & 0xFF;
	dram[addr + 1] = (w >> 8) & 0xFF;
	dram[addr + 2] = (w >> 16) & 0xFF;
	dram[addr + 3] = (w >> 24) & 0xFF;
}

uint32_t sim_dram_word(uint32_t addr){
	return dram[addr] | (dram[addr + 1] << 8) | (dram[addr + 2] << 16) | ((uint32_t)dram[addr + 3] << 24);
}

void sim_load_words(uint32_t addr, const uint32_t * words, int n){
	for(int i = 0; i < n; i++){
		sim_dram_set_word(addr + 4 * i, words[i]);
		if((addr + 4 * i) / 4 < total_number_of_instructions){
			char * s = malloc(21);
			snprintf(s, 21, "ins%-17d", i);
			instructions[(addr + 4 * i) / 4] = s;
		}
	}
}

void sim_decode_into_id_ex(uint32_t instr, uint32_t at_pc){
	if_id.IR = instr;
	if_id.PC = at_pc;
	if_id.PC_next = at_pc + 4;
	if_id.ins = "decoded             ";
	id_stage();
}

uint32_t sim_execute(uint32_t instr, uint32_t at_pc){
	sim_decode_into_id_ex(instr, at_pc);
	ex_stage();
	return ex_ma.result;
}
