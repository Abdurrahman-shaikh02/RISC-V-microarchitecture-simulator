#include "header.h"
#include "cache.h"
#include "memory.h"
#include "branch_prediction.h"
#include "control.h"



// MUST BE CALLED BEFORE ANY INIT
void configure(char * path){
	//DEFAULT CONFIGURATIONS :-
	CACHE_LEVELS = 3;
	
	l1_d.cache_size = 1024 * 2;	//2k
	l1_d.block_size = 16;
	l1_d.associativity = 2;
	l1_d.read_latency = 1;
	l1_d.write_latency = 1;
	l1_d.write_miss_policy = 1;	//write allocate
	l1_d.replacement_policy = 0;	//fifo
	l1_d.write_policy = 0;		//write through
	l1_d.level = 1;

	l1_i.cache_size = 1024 * 2;	//2k
	l1_i.block_size = 16;
	l1_i.associativity = 2;
	l1_i.read_latency = 1;
	l1_i.write_latency = 1;
	l1_i.write_miss_policy = 1;	//write allocate
	l1_i.replacement_policy = 0;	//fifo
	l1_i.write_policy = 0;		//write through
	l1_i.level = 1;


	l2.cache_size = 1024 * 8;	//8k
	l2.block_size = 32;
	l2.associativity = 4;
	l2.read_latency = 10;
	l2.write_latency = 10;
	l2.write_miss_policy = 0;	//no write allocate
	l2.replacement_policy = 0;	//fifo
	l2.write_policy = 1;		//write back
	l2.level = 2;

	l3.cache_size = 1024 * 16;	//16k
	l3.block_size = 64;
	l3.associativity = 8;
	l3.read_latency = 30;
	l3.write_latency = 30;
	l3.write_miss_policy = 0;	//no write allocate
	l3.replacement_policy = 0;	//fifo
	l3.write_policy = 1;		//write_back
	l3.level = 3;

	DRAM_READ_LATENCY = 200;
	DRAM_WRITE_LATENCY = 200;
	DRAM_SIZE = 1024 * 1024;		//1M
	
	FORWARDING_SWITCH = 1;
	BRANCH_PREDICTION_STRATEGY = 0;

	text_segment_limit = 0x0001ffff;	//128k
	//this must be l1 block aligned !!!!! is it ? i think i should remove this restriction
	//key is to not allow the same block have both code segment and data segments contents
	//the code segment boundary must not be in any block...
	rodata_segment_limit = 0x0003ffff;	//256k
	
	total_number_of_instructions = (text_segment_limit + 1) / 4;
	//-------------------------------------------------------------------------------------------------------------------------------------------------
	//read file here
	
	//-------------------------------------------------------------------------------------------------------------------------------------------------

	l1_d.cache = (cache_line *)calloc(l1_d.cache_size / 4, sizeof(cache_line));	//2 kb l1_d ram
	l1_i.cache = (cache_line *)calloc(l1_i.cache_size / 4, sizeof(cache_line));	//2 kb l1_i ram
	l2.cache = (cache_line *)calloc(l2.cache_size / 4, sizeof(cache_line));	//8 kb l2 ram
	l3.cache = (cache_line *)calloc(l3.cache_size / 4, sizeof(cache_line));	//16 kb l3 ram
	dram = (uint8_t *)calloc(DRAM_SIZE, sizeof(uint8_t));
	instructions = (char **)calloc(total_number_of_instructions, sizeof(char *));

	if(!l1_d.cache || !l1_i.cache || !l2.cache || !l3.cache || !dram){
		log_fatal("Couldnt allocate space for the cache and memory.");
		exit(1);
	}
	
	log_info("configured...");
	/*
	// use calloc buddy pls : commented this just in case you decide to make a reset function
	for(int i = 0; i < l1_d.cache_size/4; i++){
		l1_d.cache[i].addr = 0;
		l1_d.cache[i].data = 0;
		l1_d.cache[i].dirty = 0;
		l1_d.cache[i].done = 0;
		l1_d.cache[i].reference = 0;
		l1_d.cache[i].valid = 0;
	}

	for(int i = 0; i < l1_i.cache_size/4; i++){
		l1_i.cache[i].addr = 0;
		l1_i.cache[i].data = 0;
		l1_i.cache[i].dirty = 0;
		l1_i.cache[i].done = 0;
		l1_i.cache[i].reference = 0;
		l1_i.cache[i].valid = 0;
	}

	for(int i = 0; i < l2.cache_size/4; i++){
		l2.cache[i].addr = 0;
		l2.cache[i].data = 0;
		l2.cache[i].dirty = 0;
		l2.cache[i].done = 0;
		l2.cache[i].reference = 0;
		l2.cache[i].valid = 0;
	}

	for(int i = 0; i < l3.cache_size/4; i++){
		l3.cache[i].addr = 0;
		l3.cache[i].data = 0;
		l3.cache[i].dirty = 0;
		l3.cache[i].done = 0;
		l3.cache[i].reference = 0;
		l3.cache[i].valid = 0;
	}
	*/
}
