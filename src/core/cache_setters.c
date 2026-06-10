#include "header.h"
#include "cache.h"
#include "memory.h"

int set_valid(uint32_t addr, int valid, cache_line *cache, int size, int block_size, int n_way){
	//block size is in bytes btw
	uint32_t block_offset = (uint32_t)log2((double)block_size);
	uint32_t n_sets = (uint32_t)(size/(block_size * n_way));
	uint32_t index_bits = (uint32_t)log2(n_sets);
	uint32_t index = (addr << (32 - index_bits - block_offset)) >> (32 - index_bits);
	uint32_t tag = addr >> (index_bits + block_offset);

	//check all blocks in the set
	for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
		if((cache + (index * n_way * block_size / 4) + i)->tag == tag){
			for(int j = 0; j < block_size / 4; j++){
				(cache + (index * n_way * block_size / 4) + i + j)->valid = valid;
			}
			return 0;
		}
	}

	return -1;
	
}


int set_dirty(uint32_t addr, int dirty, cache_line *cache, int size, int block_size, int n_way){
	//block size is in bytes btw
	uint32_t block_offset = (uint32_t)log2((double)block_size);
	uint32_t n_sets = (uint32_t)(size/(block_size * n_way));
	uint32_t index_bits = (uint32_t)log2(n_sets);
	uint32_t index = (addr << (32 - index_bits - block_offset)) >> (32 - index_bits);
	uint32_t tag = addr >> (index_bits + block_offset);

	//check all blocks in the set
	for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
		if((cache + (index * n_way * block_size / 4) + i)->tag == tag){
			for(int j = 0; j < block_size / 4; j++){
				(cache + (index * n_way * block_size / 4) + i + j)->dirty = dirty;
			}
			return 0;
		}
	}

	return -1;
	
}


int set_done(uint32_t addr, int done, cache_line *cache, int size, int block_size, int n_way){
	//block size is in bytes btw
	uint32_t block_offset = (uint32_t)log2((double)block_size);
	uint32_t n_sets = (uint32_t)(size/(block_size * n_way));
	uint32_t index_bits = (uint32_t)log2(n_sets);
	uint32_t index = (addr << (32 - index_bits - block_offset)) >> (32 - index_bits);
	uint32_t tag = addr >> (index_bits + block_offset);

	//check all blocks in the set
	for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
		if((cache + (index * n_way * block_size / 4) + i)->tag == tag){
			for(int j = 0; j < block_size / 4; j++){
				(cache + (index * n_way * block_size / 4) + i + j)->done = done;
			}
			return 0;
		}
	}

	return -1;
	
}
