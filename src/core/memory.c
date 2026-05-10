#include "memory.h"
#include "header.h"

//assuming little endian 

static uint8_t l1[4096];	    //16KB l1 cache

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

void read_memory(){
	log_debug("read memory function commence");

	static int counter = -1;	//-1 means theres no operation carried out

	if(counter < 0){
		//reload counter based on operation;
		counter = l1_read_cycles - 1;
		log_debug("read counter set");

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
		mbr = mbr | (b0 | b1 | b2 | b3);

		//set mfc;
		log_debug("setting mfc to 1");
		mfc = 1;
		counter = -1;

		//return
		return;
	}
}

void write_memory(){
	log_debug("write memory function commence");

	static int counter = -1;	//-1 means theres no operation carried out

	if(counter < 0){
		//reload counter
		//reset mfc
		counter = l1_write_cycles - 1;
		mfc = 0;
		log_debug("write counter set");
	}

	if(counter > 0){
		counter--;
		log_debug("write counter decremented");
		return;
	}else if(counter == 0){
		if(mar > text_segment_limit){
			log_fatal("attempt to write to non text segment");
			exit(1);
		}
		//read address from mar
		//load the memory with data from mbr
		uint32_t b0 = mbr & 0x000000FF;
		uint32_t b1 = mbr & 0x0000FF00;
		uint32_t b2 = mbr & 0x00FF0000;
		uint32_t b3 = mbr & 0xFF000000;
		l1[mar + 0] = b0;
		l1[mar + 1] = b1;
		l1[mar + 2] = b2;
		l1[mar + 3] = b3;

		//set mfc;
		mfc = 1;
		counter = -1;

		//return
		return;
	}

}

void init_memory(char * path){
	//read program from file to the memory... initially l1 then later ram...
}
