#include "header.h"
#include "memory.h"
#include "cache.h"
#include "stats.h"

uint64_t cycle;

void display_cache(cache_level s){
	cache_line * cache = s.cache;
	for(int j = 0; j < s.cache_size / 4; j += 4){
		printf("line no %d: ", j);
		for(int i = 0; i < 4; i++){
			printf("%08x : %d, %d, %d, %016lx |", cache[j + i].addr, cache[j + i].valid, cache[j + i].dirty, cache[j + i].done, cache[j + i].reference);
		}
		printf("\n");
	}
}

void display(){
	printf("l1_i:\n");
	//display_cache(l1_i);
	printf("l1_d:\n");
	display_cache(l1_d);
	printf("l2:\n");
	display_cache(l2);
	printf("l3:\n");
	display_cache(l3);
}

void configure(char * path);

int main(){
	configure(NULL);
	
	char s = 0;
	uint32_t addr = 0;
	cycle = 0;
	while(scanf("%c %x", &s, &addr) == 2){
		switch(s){
			case 'w':
				//write
				printf("%ld : %d\n", ++cycle, set_write_counter(addr));
				complete_write(addr);
				break;
			case 'r':
				//read
				printf("%ld : %d\n", ++cycle, set_read_counter(addr));
				complete_read(addr);
				break;
			case 'i':
				//read instruction
				printf("%ld : %d\n", ++cycle, set_read_i_counter(addr));
				complete_read_i(addr);
				break;
			default:
				printf("Illegal operation");
				exit(1);
		}
		getchar();
	}

	//display();
	display_stats(NULL);

	return 1;
}


// SUPPOSED TO WORK ONLY FOR CACHESIM !!!!!!
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
	
	//-------------------------------------------------------------------------------------------------------------------------------------------------

	l1_d.cache = (cache_line *)calloc(l1_d.cache_size / 4, sizeof(cache_line));	//2 kb l1_d ram
	l1_i.cache = (cache_line *)calloc(l1_i.cache_size / 4, sizeof(cache_line));	//2 kb l1_i ram
	l2.cache = (cache_line *)calloc(l2.cache_size / 4, sizeof(cache_line));	//8 kb l2 ram
	l3.cache = (cache_line *)calloc(l3.cache_size / 4, sizeof(cache_line));	//16 kb l3 ram

	if(!l1_d.cache || !l1_i.cache || !l2.cache || !l3.cache){
		log_fatal("Couldnt allocate space for the cache");
		exit(1);
	}
	
	log_info("configured...");
}
