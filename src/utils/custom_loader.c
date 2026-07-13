#include "header.h"
#include "memory.h"
#include "custom_loader.h"

//allocates all instructions strings, if theres no instruction, it fills with 20 spaces
void custom_loader_i(char *path){
	FILE *f = fopen(path, "r");

	if (f == NULL) {
		log_fatal("failed to open memory image file");
		exit(1);
	}

	char line[256];

	uint32_t addr = 0;
	uint32_t instr_index = 0;

	while (fgets(line, sizeof(line), f)) {

		/*
			expected:
			[151] 0x00025c   000ff117   auipc r2, 0xFF
		*/

		char *p = line;

		// -----------------------------------------
		// skip: [151]
		// -----------------------------------------
		char *bracket = strchr(p, ']');

		if (bracket == NULL)
			continue;

		p = bracket + 1;

		// -----------------------------------------
		// skip whitespace
		// -----------------------------------------
		while (*p == ' ' || *p == '\t')
			p++;

		// -----------------------------------------
		// skip address
		// -----------------------------------------
		char *space = strchr(p, ' ');

		if (space == NULL)
			continue;

		p = space;

		while (*p == ' ' || *p == '\t')
			p++;

		// -----------------------------------------
		// parse opcode
		// -----------------------------------------
		uint32_t opcode;

		if (sscanf(p, "%x", &opcode) != 1)
			continue;

		// move past opcode
		while (*p && *p != ' ' && *p != '\t')
			p++;

		while (*p == ' ' || *p == '\t')
			p++;

		// -----------------------------------------
		// bounds checks
		// -----------------------------------------
		if (addr + 3 >= text_segment_limit) {
			log_fatal("program too large for memory");
			fclose(f);
			exit(1);
		}

		if (instr_index >= total_number_of_instructions) {
			log_fatal("too many instructions");
			fclose(f);
			exit(1);
		}

		// -----------------------------------------
		// store opcode little endian
		// -----------------------------------------
		dram[addr + 0] = (opcode >> 0)  & 0xFF;
		dram[addr + 1] = (opcode >> 8)  & 0xFF;
		dram[addr + 2] = (opcode >> 16) & 0xFF;
		dram[addr + 3] = (opcode >> 24) & 0xFF;

		// -----------------------------------------
		// allocate 20-char instruction string
		// -----------------------------------------
		instructions[instr_index] = malloc(21);

		if (instructions[instr_index] == NULL) {
			log_fatal("malloc failed");
			fclose(f);
			exit(1);
		}

		// fill all 20 chars with spaces
		memset(instructions[instr_index], ' ', 20);

		// null terminator
		instructions[instr_index][20] = '\0';

		// copy manually without copying '\0'
		for (int i = 0; i < 20; i++) {

			if (p[i] == '\0' || p[i] == '\n')
				break;

			instructions[instr_index][i] = p[i];
		}

		addr += 4;
		instr_index++;
	}

	fclose(f);

	log_debug("memory initialization complete...");
}

void custom_loader_d(char *path){

}

void custom_loader(char * instruction_file, char * data_file){
	//open the memory access histroy file... for writing
	memory_access_history_file_d = fopen("data_trace.txt", "w");
	memory_access_history_file_i = fopen("instruction_trace.txt", "w");
	if(memory_access_history_file_d == NULL || memory_access_history_file_i == NULL){
		log_fatal("trace files couldnt be created");
		exit(1);
	}
	
	custom_loader_i(instruction_file);
	custom_loader_d(data_file);
}

