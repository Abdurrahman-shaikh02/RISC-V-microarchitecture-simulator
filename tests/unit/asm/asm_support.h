// Shared helpers for the assembler unit tests
#ifndef ASM_SUPPORT_H
#define ASM_SUPPORT_H

#include "test.h"
#include "rv32.h"
#include "assembler.h"

void free_symbol_table();	// src/assembler/core/symbol_table.c, not in assembler.h

typedef struct {
	char * tok[MAX_TOKENS_IN_A_LINE];
	char storage[MAX_TOKENS_IN_A_LINE][MAX_TOKEN_CHAR];
	char line[256];
	int n;		// get_tokens() result
} tokens;

// runs get_tokens on a copy of line (get_tokens modifies its input)
// the result lives in a small static pool (valid for the next 15 calls)
static tokens * tokenize(const char * line){
	static tokens pool[16];
	static int next = 0;
	tokens * t = &pool[next++ % 16];
	memset(t, 0, sizeof(*t));
	CURRENT_LEVEL = 6;
	for(int i = 0; i < MAX_TOKENS_IN_A_LINE; i++) t->tok[i] = t->storage[i];
	snprintf(t->line, sizeof(t->line), "%s", line);
	t->n = get_tokens(t->line, t->tok, MAX_TOKENS_IN_A_LINE, MAX_TOKEN_CHAR);
	return t;
}

// tokenizes and encodes one instruction located at address
static uint32_t assemble_line(const char * line, int address){
	tokens * t = tokenize(line);
	if(t->n <= 0) return 0xDEAD0000u | (uint32_t)(t->n & 0xFF);
	return (uint32_t)get_opcode(t->tok, address);
}

#endif
