#include "header.h"
#include "memory.h"


uint32_t text_segment_limit = 0x7FF;	//preferably something divisible by 4... :)
#define total_number_of_instructions (0x7FF + 1)/4	//total text bytes / 4
uint8_t mfc_i = 1;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
uint32_t mar_i;
uint32_t mbr_i;

uint8_t mfc = 1;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
uint32_t mar;
uint32_t mbr;

//assuming little endian 

char * instructions[total_number_of_instructions];
uint8_t l1[4096];	    //16KB l1 cache

//l2 
//
//then ram
//

void read_memory_i(){
	//remember to check for write permission (text segment limit)
	//need changes in write_memory as well... !!!!
	log_debug("read instruction function commence");

	static int counter = -1;	//-1 means theres no operation carried out

	if(counter < 0){
		//reload counter based on operation;
		counter = 0;
		if(counter == -1){
			//Structural hazard
			log_info("Uh oh ! Structural hazard...");
			STRUCTURAL_HAZARD_STALL = 1;
			return;
		}

		log_debug("read instruction counter set");

		//reset mfc
		mfc_i = 0;
	}

	if(counter > 0){
		counter--;
		log_debug("read instruction counter decremented");
		return;
	}else if(counter == 0){
		log_debug("loading mbr_i with [mar_i]");
		//read address from mar load into mbr
		if(mar_i > text_segment_limit){
			log_fatal("attempt to fetch instruction from non text segment");
			exit(1);
		}

		if(mar_i % 4 != 0){
			//this is an unaligned access
			log_fatal("Unaligned instruction word access");
			exit(1);
		}

		uint32_t b0 = l1[mar_i + 0];
		uint32_t b1 = l1[mar_i + 1] << 8;
		uint32_t b2 = l1[mar_i + 2] << 16;
		uint32_t b3 = l1[mar_i + 3] << 24;
		mbr_i = 0;
		mbr_i = mbr_i | (b0 | b1 | b2 | b3);

		//set mfc;
		log_debug("setting mfc to 1");
		mfc_i = 1;
		counter = -1;

		//return
		return;
	}
}


void read_memory(uint32_t opcode){
	log_debug("read memory function commence");

	static int counter = -1;	//-1 means theres no operation carried out

	if(counter < 0){
		//reload counter based on operation;
		counter = l1_read_cycles - 1;
		
		if(counter == -1){
			//Structural hazard
			log_info("Uh oh ! Structural hazard...");
			STRUCTURAL_HAZARD_STALL = 1;
			return;
		}

		log_debug("read data counter set");
		//reset mfc
		mfc = 0;
	}

	if(counter > 0){
		counter--;
		log_debug("read counter decremented");
		return;
	}else if(counter == 0){
		log_debug("loading mbr with [mar]");
		//read address from mar load into mbr
		uint32_t b0 = l1[mar + 0];
		uint32_t b1 = l1[mar + 1] << 8;
		uint32_t b2 = l1[mar + 2] << 16;
		uint32_t b3 = l1[mar + 3] << 24;
		mbr = 0;

		switch(opcode){
			case 0b000:
				//signed byte
				mbr = mbr | ((int32_t)(b0 << 24) >> 24);	//handle sign extension
				break;
			case 0b001:
				//signed halfword
				if(mar % 2 != 0){
					//this is an unaligned access
					log_fatal("Unaligned halfword access");
					exit(1);
				}
				mbr = mbr | (((int32_t)((b0 | b1) << 16)) >> 16);
				break;
			case 0b010:
				//word
				if(mar % 4 != 0){
					//this is an unaligned access
					log_fatal("Unaligned word access");
					exit(1);
				}
				mbr = mbr | (b0 | b1 | b2 | b3);
				break;
			case 0b011:
				//unsigned byte
				mbr = mbr | b0;
				break;
			case 0b100:
				//unsigned halfword
				if(mar % 2 != 0){
					//this is an unaligned access
					log_fatal("Unaligned halfword access");
					exit(1);
				}
				mbr = mbr | b0 | b1;
				break;
			default:
				log_fatal("invalid memory access opcode.");
				exit(1);
		}

		//set mfc;
		log_debug("setting mfc to 1");
		mfc = 1;
		counter = -1;

		//return
		return;
	}
}


void write_memory(uint32_t opcode){
	log_debug("write memory function commence");

	static int counter = -1;	//-1 means theres no operation carried out

	if(counter < 0){
		//reload counter
		//reset mfc
		counter = l1_write_cycles - 1;
		if(counter == -1){
			//Structural hazard
			log_info("Uh oh ! Structural hazard...");
			STRUCTURAL_HAZARD_STALL = 1;
			return;
		}

		mfc = 0;
		log_debug("write counter set");
	}

	if(counter > 0){
		counter--;
		log_debug("write counter decremented");
		return;
	}else if(counter == 0){
		if(mar <= text_segment_limit){
			log_fatal("attempt to write to text segment");
			exit(1);
		}
		//read address from mar
		//load the memory with data from mbr
		uint8_t b0 = (uint8_t)(mbr & 0x000000FF);
		uint8_t b1 = (uint8_t)((mbr & 0x0000FF00) >> 8);
		uint8_t b2 = (uint8_t)((mbr & 0x00FF0000) >> 16);
		uint8_t b3 = (uint8_t)((mbr & 0xFF000000) >> 24);

		switch(opcode){
			case 0b000:
				//byte
				l1[mar + 0] = b0;
				break;
			case 0b001:
				//halfword
				if(mar % 2 != 0){
					//this is an unaligned access
					log_fatal("Unaligned halfword access");
					exit(1);
				}
				l1[mar + 0] = b0;
				l1[mar + 1] = b1;
				break;
			case 0b010:
				//word
				if(mar % 4 != 0){
					//this is an unaligned access
					log_fatal("Unaligned word access");
					exit(1);
				}
				l1[mar + 0] = b0;
				l1[mar + 1] = b1;
				l1[mar + 2] = b2;
				l1[mar + 3] = b3;
				break;
			default:
				log_fatal("invalid memory opcode");
				exit(1);
		}

		//set mfc;
		mfc = 1;
		counter = -1;

		//return
		return;
	}

}


void init_memory(char * path){
	// read program from file into memory
	// expected format per line:
	// 00000000
	// (32-bit instruction/data in hex)

	FILE *f = fopen(path, "r");
	if(f == NULL){
		log_fatal("failed to open memory image file");
		exit(1);
	}

	char line[64];
	uint32_t addr = 0;

	while(fgets(line, sizeof(line), f)){
		uint32_t value;

		// parse 32-bit hex value
		if(sscanf(line, "%x", &value) != 1){
			continue;	// skip invalid lines
		}

		// bounds check
		if(addr + 3 >= text_segment_limit){
			log_fatal("program too large for memory");
			fclose(f);
			exit(1);
		}

		// store as little endian
		l1[addr + 0] = (value >> 0)  & 0xFF;
		l1[addr + 1] = (value >> 8)  & 0xFF;
		l1[addr + 2] = (value >> 16) & 0xFF;
		l1[addr + 3] = (value >> 24) & 0xFF;

		addr += 4;
	}

	fclose(f);
	log_debug("memory initialization complete...");
}

void display_memory(){
	for(int i = 0; i < sizeof(l1); i+=16){
		printf("[%08x] :", i);
		for(int j = 0; j < 16; j++){
			printf(" %02x", l1[i+j]);
		}
		printf("\n");
	}
}

void init_memory_i(char *path)
{
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
		l1[addr + 0] = (opcode >> 0)  & 0xFF;
		l1[addr + 1] = (opcode >> 8)  & 0xFF;
		l1[addr + 2] = (opcode >> 16) & 0xFF;
		l1[addr + 3] = (opcode >> 24) & 0xFF;

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


void free_instructions(void)
{
	for (uint32_t i = 0; i < total_number_of_instructions; i++) {

		if (instructions[i] != NULL) {
			free(instructions[i]);
			instructions[i] = NULL;
		}
	}
}

void display_instructions(){
	for(int i = 0; i < total_number_of_instructions; i++){
		printf("%d : %s\n", i, instructions[i]);
	}
}


