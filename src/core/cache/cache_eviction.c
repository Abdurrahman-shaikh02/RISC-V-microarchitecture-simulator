#include "header.h"
#include "cache.h"
#include "memory.h"


//replace() and different policies...


//EXPECTS BLOCK ALIGNED ADDRESSES...
void copy_from_mem(uint32_t addr, cache_line * cache, int n_words){
	for(int i = 0; i < n_words; i++){
		cache[i].addr = addr;
		addr += 4;
	}

}

//in reality, the lower level should have a data in order for us to replace that same data in the upper cache...
//but we are not going to check the lower cache for a hit because while calling we will ensure to call it only when the lower cache has the data
//the return value is nothing but the write penalty that me be caused due to writebacks before evicting an existing block
int evict(uint32_t addr, cache_level s){
	switch(s.replacement_policy){
		case 0:
			return evict_fifo(addr, s);
		default:
			log_fatal("Invalid replacement policy");
			exit(1);
	}
}

int evict_fifo(uint32_t addr, cache_level s){
	int found = find_block(addr, s);
	cache_line * cache = s.cache;
	int size = s.cache_size;
	int block_size = s.block_size;
	int n_way = s.associativity;

	//block size is in bytes btw
	uint32_t block_offset = (uint32_t)log2((double)block_size);
	uint32_t n_sets = (uint32_t)(size/(block_size * n_way));
	uint32_t index_bits = (uint32_t)log2(n_sets);
	uint32_t index = (addr << (32 - index_bits - block_offset)) >> (32 - index_bits);
	uint32_t tag = get_tag(addr, size, block_size, n_way);

	if(found == 0){
		//loop to see if there are any invalid locations first...
		//if yes place there...
		//check all blocks in the set for an valid = 0
		//we keep this segment here because we want to keep the implementations of different policies totally isolated from each other...
		for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
			cache_line * line;
			line = (cache + (index * n_way * block_size / 4) + i);
			if(line->valid == 0){
				//first set the address
				//copy from the start of the block so mask the block offset bits
				uint32_t block_addr = (addr >> block_offset) << block_offset;
				copy_from_mem(block_addr, line, block_size / 4);
				
				//then set the flags...
				set_valid(addr, 1, s);
				set_reference(addr, cycle, s);
				set_dirty(addr, 0, s);
				set_done(addr, 0, s);

				return 0;	//no write back stalls so ret 0
			}
		}

		//if an invalid block is not found:
		//find the lowest reference, see if it is dirty(for writeback) if yes then invoke write on the right level
		//save the stall count to return later...
		//and if not then just copy from memory
		//return the write stall_count
	}else{
		log_fatal("theres no space for eviction but evict was called... shouldnt happen");
		exit(1);
	}
}
