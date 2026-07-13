#include "header.h"
#include "memory.h"
#include "stats.h"
#include "cache.h"

FILE * memory_access_history_file_d;
FILE * memory_access_history_file_i;

uint32_t text_segment_limit;	//THIS IS THE LAST VALID ADDRESS FOR THE CODE SEGMENT (eg : 7ff... although unaligned... it holds the last byte of the code segment)
uint32_t rodata_segment_limit;
uint32_t total_number_of_instructions;
uint8_t mfc_i = 1;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
uint32_t mar_i;
uint32_t mbr_i;

uint8_t mfc = 1;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
uint32_t mar;
uint32_t mbr;

//assuming little endian 

char ** instructions;
uint8_t * dram;	    //16KB 


void read_memory_i(int cancel){
	//if cancel is non zero then reset the read memory i function
	
	static int counter = -1;	//-1 means theres no operation carried out

	if(cancel){
		log_info("Cancelling ongoing memory instruction read !");
		//reset internal counter
		counter = -1;
		//set mfc_i
		mfc_i = 1;
		//completing any pending actions
		complete_read_i(mar_i);

		return;
	}

	log_debug("read instruction function commence");

	if(counter < 0){
		//reload counter based on operation;
		counter = set_read_i_counter(mar_i) - 1;	//returns total required no of cycles, so - 1 to convert to no of extra stalls

		if(counter == -1){
			//Structural hazard
			log_info("Uh oh ! Structural hazard...");

			n_structural_hazards++;		//stat

			STRUCTURAL_HAZARD_STALL = 1;
			return;
		}
		
		n_read_i_stalls += counter;	//stat

		log_debug("read instruction counter set");

		//reset mfc
		mfc_i = 0;
	}

	if(counter > 0){
		counter--;
		log_debug("read instruction counter decremented");
		return;
	}else if(counter == 0){
		complete_read_i(mar_i);
		n_instruction_reads++;	//stat

		log_debug("loading mbr_i with [mar_i]");
		//read address from mar load into mbr
		if(mar_i > text_segment_limit || mar_i < 0){
			log_fatal("attempt to fetch instruction from non text segment");
			exit(1);
		}

		if(mar_i % 4 != 0){
			//this is an unaligned access
			log_fatal("Unaligned instruction word access");
			exit(1);
		}

		uint32_t b0 = dram[mar_i + 0];
		uint32_t b1 = dram[mar_i + 1] << 8;
		uint32_t b2 = dram[mar_i + 2] << 16;
		uint32_t b3 = dram[mar_i + 3] << 24;
		mbr_i = 0;
		mbr_i = mbr_i | (b0 | b1 | b2 | b3);

		fprintf(memory_access_history_file_i, "i %#08x\n", mar_i);		//memory access history file
	
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
		counter = set_read_counter(mar) - 1;	//return the total no of cycles required so -1 to convert to no of extra stalls
		
		if(counter == -1){
			//Structural hazard
			log_info("Uh oh ! Structural hazard...");

			n_structural_hazards++;		//stat

			STRUCTURAL_HAZARD_STALL = 1;
			return;
		}
		
		n_read_d_stalls += counter;	//stat

		log_debug("read data counter set");
		//reset mfc
		mfc = 0;
	}

	if(counter > 0){
		counter--;
		log_debug("read counter decremented");
		return;
	}else if(counter == 0){
		complete_read(mar);
		n_reads++;	//stat

		log_debug("loading mbr with [mar]");
		//read address from mar load into mbr
		uint32_t b0 = dram[mar + 0];
		uint32_t b1 = dram[mar + 1] << 8;
		uint32_t b2 = dram[mar + 2] << 16;
		uint32_t b3 = dram[mar + 3] << 24;
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

		fprintf(memory_access_history_file_d, "r %#08x\n", mar);		//memory access history file

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
		counter = set_write_counter(mar) - 1;	//return total no of cycles required so -1 to convert to no of extra stalls

		if(counter == -1){
			//Structural hazard
			log_info("Uh oh ! Structural hazard...");

			n_structural_hazards++;		//stat

			STRUCTURAL_HAZARD_STALL = 1;
			return;
		}
		
		n_write_d_stalls += counter;	//stat
		
		mfc = 0;
		log_debug("write counter set");
	}

	if(counter > 0){
		counter--;
		log_debug("write counter decremented");
		return;
	}else if(counter == 0){
		complete_write(mar);

		n_writes++;	//stat

		if(mar <= text_segment_limit){
			log_fatal("attempt to write to text segment");
			exit(1);
		}

		if(mar <= rodata_segment_limit){
			log_fatal("attempt to write to rodata segment");
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
				dram[mar + 0] = b0;
				break;
			case 0b001:
				//halfword
				if(mar % 2 != 0){
					//this is an unaligned access
					log_fatal("Unaligned halfword access");
					exit(1);
				}
				dram[mar + 0] = b0;
				dram[mar + 1] = b1;
				break;
			case 0b010:
				//word
				if(mar % 4 != 0){
					//this is an unaligned access
					log_fatal("Unaligned word access");
					exit(1);
				}
				dram[mar + 0] = b0;
				dram[mar + 1] = b1;
				dram[mar + 2] = b2;
				dram[mar + 3] = b3;
				break;
			default:
				log_fatal("invalid memory opcode");
				exit(1);
		}

		fprintf(memory_access_history_file_d, "w %#08x\n", mar);		//memory access history file

		//set mfc;
		mfc = 1;
		counter = -1;

		//return
		return;
	}

}

void display_memory(char * path){
	FILE * fd = NULL;

	if(path == NULL){
		fd = stdout;
	}else{
		fd = fopen(path, "w");
	}

	if(fd == NULL){
		log_fatal("Couldnt open file for memory dump.");
		exit(1);
	}

	for(int i = 0; i < DRAM_SIZE; i+=32){
		fprintf(fd, "[%08x] :", i);
		for(int j = 0; j < 32; j++){
			fprintf(fd, " %02x", dram[i+j]);
		}
		fprintf(fd, "\n");
	}

	if(path != NULL){
		fclose(fd);
	}
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


