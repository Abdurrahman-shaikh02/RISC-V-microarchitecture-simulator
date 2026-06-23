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
		case 1:
			return evict_lru(addr, s);
		default:
			log_fatal("Invalid replacement policy");
			exit(1);
	}
}

int evict_lru(uint32_t addr, cache_level s){
	//its literally the same as evict fifo... find the lowest reference block...
	//the touch has to do its job actually... so the difference is in touch function
	
	int found = find_block(addr, s);	//just checking :)
	if(found == 1){
		log_fatal("already exists...no eviction needed... shouldnt be called if it exists...");
		exit(1);
	}

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

		log_debug("an invalid block was not found in the set");
		//if an invalid block is not found:

		//find the lowest reference, see if it is done...(if not... find the next smallest)
		uint64_t min_ref = INT64_MAX;
		int min_ref_index = -1;	//the block number in the set...
		int skip_index = -1;	//theres only one possible block in the set that could be NOT marked as done ! -> ASSUMPTION
		cache_line * line;
		for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
			//loop over every block in the set
			if(i == skip_index) continue;
			line = (cache + (index * n_way * block_size / 4) + i);
			if(line->reference < min_ref){
				min_ref = line->reference;
				min_ref_index = i;
			}
		}

		line = (cache + (index * n_way * block_size / 4) + min_ref_index);
		if(line->done == 0){
			//the lowest reference block is not done
			log_debug("oops the eviction block is not done finding the next smallest reference");
			skip_index = min_ref_index;
			min_ref_index = -1;
			min_ref = INT64_MAX;
			//do the search again but skip the index found earlier
			for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
				//loop over every block in the set
				if(i == skip_index) continue;
				line = (cache + (index * n_way * block_size / 4) + i);
				if(line->reference < min_ref){
					min_ref = line->reference;
					min_ref_index = i;
				}
			}
		}

		line = (cache + (index * n_way * block_size / 4) + min_ref_index);	//line now points to the first address of the block to be evicted

		//then see if it is writeback 
		if(s.write_policy == 1){
			int stall_count = 0;

			//if yes then see if it is dirty... if yes invoke write on the right level
			if(line->dirty == 1){
				//write the to be evicted block into the lower level(use s.level to dispatch proper write method)
				switch (s.level) {
					case 1:
						if(CACHE_LEVELS > 1){
							//write to l2, count in stall_count
							log_info("eviction requires writing back to l2");
							stall_count = l2_write(line->addr);

							//finishing the above write operation...
							complete_below(line->addr, l2);
						}else{
							log_info("eviction requires writing back to ram");
							stall_count = DRAM_WRITE_LATENCY;
						}
						break;
					case 2: if(CACHE_LEVELS > 2){
							//write to l3, count int stall_count
							log_info("eviction requires writing back to l3");
							stall_count = l3_write(line->addr);

							//finishing the above write operation...
							complete_below(line->addr, l3);
						}else{
							log_info("eviction requires writing back to ram");
							stall_count = DRAM_WRITE_LATENCY;
						}
						break;
					case 3:
						//just add dram write latency to count
						log_info("eviction requires writing back to ram");
						stall_count = DRAM_WRITE_LATENCY;
						break;
					default:
						log_fatal("Illegal cache level in evict...");
						exit(1);
				}
			}
			

			uint32_t block_addr = (addr >> block_offset) << block_offset;
			copy_from_mem(block_addr, line, block_size / 4);

			//then set the flags...
			set_valid(addr, 1, s);
			set_reference(addr, cycle, s);
			set_dirty(addr, 0, s);
			set_done(addr, 0, s);

			//if block is not dirty stall_count will be zero
			return stall_count;
		}else{
			//and if its write through we assume it has been written to lower level...
			//and then just copy from memory
			uint32_t block_addr = (addr >> block_offset) << block_offset;
			copy_from_mem(block_addr, line, block_size / 4);

			//then set the flags...
			set_valid(addr, 1, s);
			set_reference(addr, cycle, s);
			set_dirty(addr, 0, s);
			set_done(addr, 0, s);

			return 0;	//because no write backs had to be made due to write through
		}
	}else{
		log_fatal("theres no space for eviction but evict was called... shouldnt happen");
		exit(1);
	}
}

int evict_fifo(uint32_t addr, cache_level s){
	int found = find_block(addr, s);	//just checking :)
	if(found == 1){
		log_fatal("already exists...no eviction needed... shouldnt be called if it exists...");
		exit(1);
	}

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

		log_debug("an invalid block was not found in the set");
		//if an invalid block is not found:

		//find the lowest reference, see if it is done...(if not... find the next smallest)
		uint64_t min_ref = INT64_MAX;
		int min_ref_index = -1;	//the block number in the set...
		int skip_index = -1;	//theres only one possible block in the set that could be NOT marked as done ! -> ASSUMPTION
		cache_line * line;
		for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
			//loop over every block in the set
			if(i == skip_index) continue;
			line = (cache + (index * n_way * block_size / 4) + i);
			if(line->reference < min_ref){
				min_ref = line->reference;
				min_ref_index = i;
			}
		}

		line = (cache + (index * n_way * block_size / 4) + min_ref_index);
		if(line->done == 0){
			//the lowest reference block is not done
			log_debug("oops the eviction block is not done finding the next smallest reference");
			skip_index = min_ref_index;
			min_ref_index = -1;
			min_ref = INT64_MAX;
			//do the search again but skip the index found earlier
			for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
				//loop over every block in the set
				if(i == skip_index) continue;
				line = (cache + (index * n_way * block_size / 4) + i);
				if(line->reference < min_ref){
					min_ref = line->reference;
					min_ref_index = i;
				}
			}
		}

		line = (cache + (index * n_way * block_size / 4) + min_ref_index);	//line now points to the first address of the block to be evicted

		//then see if it is writeback 
		if(s.write_policy == 1){
			int stall_count = 0;

			//if yes then see if it is dirty... if yes invoke write on the right level
			if(line->dirty == 1){
				//write the to be evicted block into the lower level(use s.level to dispatch proper write method)
				switch (s.level) {
					case 1:
						if(CACHE_LEVELS > 1){
							//write to l2, count in stall_count
							log_info("eviction requires writing back to l2");
							stall_count = l2_write(line->addr);

							//finishing the above write operation...
							complete_below(line->addr, l2);
						}else{
							log_info("eviction requires writing back to ram");
							stall_count = DRAM_WRITE_LATENCY;
						}
						break;
					case 2: if(CACHE_LEVELS > 2){
							//write to l3, count int stall_count
							log_info("eviction requires writing back to l3");
							stall_count = l3_write(line->addr);

							//finishing the above write operation...
							complete_below(line->addr, l3);
						}else{
							log_info("eviction requires writing back to ram");
							stall_count = DRAM_WRITE_LATENCY;
						}
						break;
					case 3:
						//just add dram write latency to count
						log_info("eviction requires writing back to ram");
						stall_count = DRAM_WRITE_LATENCY;
						break;
					default:
						log_fatal("Illegal cache level in evict...");
						exit(1);
				}
			}
			

			uint32_t block_addr = (addr >> block_offset) << block_offset;
			copy_from_mem(block_addr, line, block_size / 4);

			//then set the flags...
			set_valid(addr, 1, s);
			set_reference(addr, cycle, s);
			set_dirty(addr, 0, s);
			set_done(addr, 0, s);

			//if block is not dirty stall_count will be zero
			return stall_count;
		}else{
			//and if its write through we assume it has been written to lower level...
			//and then just copy from memory
			uint32_t block_addr = (addr >> block_offset) << block_offset;
			copy_from_mem(block_addr, line, block_size / 4);

			//then set the flags...
			set_valid(addr, 1, s);
			set_reference(addr, cycle, s);
			set_dirty(addr, 0, s);
			set_done(addr, 0, s);

			return 0;	//because no write backs had to be made due to write through
		}
	}else{
		log_fatal("theres no space for eviction but evict was called... shouldnt happen");
		exit(1);
	}
}
