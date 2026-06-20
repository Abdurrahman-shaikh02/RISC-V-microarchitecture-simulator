#include "header.h"
#include "memory.h"
#include "cache.h"
#include <stdio.h>

uint64_t cycle = 0x99;
int CACHE_LEVELS = 3;

cache_level l1_i, l1_d, l2, l3;
int DRAM_READ_LATENCY, DRAM_WRITE_LATENCY, DRAM_SIZE;

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

int main(){
	configure_cache();
	
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
			default:
				printf("Illegal operation");
				exit(1);
		}
		getchar();
	}

	//display();
	
	return 1;
}
