#include "header.h"
#include "cache.h"
#include "memory.h"




void configure(char * path){
	//DEFAULT CONFIGURATIONS :-
	
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
	l2.read_latency = 2;
	l2.write_latency = 2;
	l2.write_miss_policy = 0;	//no write allocate
	l2.replacement_policy = 0;	//fifo
	l2.write_policy = 1;		//write back
	l2.level = 2;

	l3.cache_size = 1024 * 16;	//16k
	l3.block_size = 64;
	l3.associativity = 8;
	l3.read_latency = 3;
	l3.write_latency = 3;
	l3.write_miss_policy = 0;	//no write allocate
	l3.replacement_policy = 0;	//fifo
	l3.write_policy = 1;		//write_back
	l3.level = 3;

	DRAM_READ_LATENCY = 4;
	DRAM_WRITE_LATENCY = 4;
	DRAM_SIZE = 32 * 1024;		//32k
	//-------------------------------------------------------------------------------------------------------------------------------------------------
	//read file here
	
	//dram size too
	//-------------------------------------------------------------------------------------------------------------------------------------------------

	l1_d.cache = (cache_line *)calloc(l1_d.cache_size / 4, sizeof(cache_line));	//2 kb l1_d ram
	l1_i.cache = (cache_line *)calloc(l1_i.cache_size / 4, sizeof(cache_line));	//2 kb l1_i ram
	l2.cache = (cache_line *)calloc(l2.cache_size / 4, sizeof(cache_line));	//8 kb l2 ram
	l3.cache = (cache_line *)calloc(l3.cache_size / 4, sizeof(cache_line));	//16 kb l3 ram
	//allocate ram here too

	if(!l1_d.cache || !l1_i.cache || !l2.cache || !l3.cache){
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
