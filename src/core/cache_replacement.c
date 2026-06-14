#include "header.h"
#include "cache.h"
#include "memory.h"


//replace() and different policies...

int replace(uint32_t addr, cache_line * cache, int size, int block_size, int n_way){
	switch(REPLACEMENT_POLICY){
		case 0: replace_fifo(addr, cache, size, block_size, n_way);
			return;
		case 1: replace_lru(addr, cache, size, block_size, n_way);
			return;
		default:
			log_fatal("Invalid replacement strategy.");
			exit(1);
	}
}


void copy_from_mem(uint32_t addr, cache_line * cache, int n_words){
	//expecting BLOCK ALIGNED addresses...
	for(int i = 0; i < n_words; i++){
		//deal with little endian...
	}

}


//WE WILL BE ASSUMING THAT THERE IS ATLEAST ONE VALID BLOCK AVAILABLE FOR US TO REPLACE !
//IT HAS BEEN VERIFIED BY find_block BEFORE THE CALL TO REPLACE
int replace_fifo(uint32_t addr, cache_line * cache, int size, int block_size, int n_way){
	//first find the block with a valid bit = 0, if not then find a block that has the least number in reference...
	//then replace that block from the memory...

}

int replace_lru(uint32_t addr, cache_line * cache, int size, int block_size, int n_way){

}


