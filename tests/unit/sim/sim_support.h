// Shared helpers for the simulator unit tests.
//
// The project headers have no include guards, so tests must include ONLY this
// header (it includes every simulator header exactly once, in a working order).

#ifndef SIM_SUPPORT_H
#define SIM_SUPPORT_H

#include "test.h"
#include "rv32.h"

#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "memory_configurables.h"
#include "internal_memory.h"
#include "cache.h"
#include "stats.h"
#include "alu.h"
#include "branch_prediction.h"
#include "static_branch_prediction.h"
#include "dynamic_branch_prediction.h"
#include "forwarding.h"
#include "configure.h"
#include "json_loader.h"
#include "custom_loader.h"
#include "elf_loader.h"
#include "utility.h"
#include "simulator.h"

#define BLANK_INS "                    "

// silence the logger (same thing main.c does without -l)
void sim_quiet(void);

// configure(NULL) (default machine) + quiet logs + empty pipeline
// cache_levels: -1 ideal memory, 0 dram only, 1..3 cache levels
void sim_setup(int cache_levels);

// clears every pipeline register, hazard flag and the register file
void sim_reset_pipeline(void);

// stores words little endian at addr (and a dummy instruction string for each word)
void sim_load_words(uint32_t addr, const uint32_t * words, int n);
uint32_t sim_dram_word(uint32_t addr);
void sim_dram_set_word(uint32_t addr, uint32_t value);

// decodes ir (as if fetched from pc) through the real ID stage into id_ex
void sim_decode_into_id_ex(uint32_t ir, uint32_t pc);

// decode + execute one instruction, returns ex_ma.result
uint32_t sim_execute(uint32_t ir, uint32_t pc);

// the encodings the project assembler emits for the system instructions
#define ENC_ECALL  0x00000073u
#define ENC_EBREAK_PROJECT 0x02000073u	// funct7 = 1 (what ./assemble emits)
#define ENC_EBREAK_STANDARD 0x00100073u	// imm = 1 (what the RISC-V spec / gcc emit)

#endif
