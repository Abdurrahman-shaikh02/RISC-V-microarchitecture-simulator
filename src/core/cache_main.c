#include "header.h"
#include "cache.h"
#include "memory.h"

int set_read_counter(){
	//find the block, if 0 look into further levels... if 1 then make changes and return the counter...
	//if -1 set a structural hazard stall
}


int find_block(uint32_t addr, cache_line *cache, int size, int block_size, int n_way){
	//index set, and the iterate over every block in that set to see if the tag exists...(tag is same for all words in the block)
	//if not there see if there is atleast one block that we can replce (done bit is set)
	uint32_t block_offset = (uint32_t)log2((double)block_size);
	uint32_t n_sets = (uint32_t)(size/(block_size * n_way));
	uint32_t index_bits = (uint32_t)log2(n_sets);
	uint32_t index = (addr << (32 - index_bits - block_offset)) >> (32 - index_bits);
	uint32_t tag = get_tag(addr, size, block_size, n_way);

	
	//check all blocks in the set
	for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
		cache_line * line;
		line = (cache + (index * n_way * block_size / 4) + i);
		uint32_t line_tag = get_tag(line->addr, size, block_size, n_way);
		if(line_tag == tag && line->done == 1){
			log_info("cache hit !");
			return 1;
		}else if(line_tag == tag && line->done == 0){
			//structural hazard
			log_info("structural hazard waow thats rare... actually impossible... somethings wrong bud !");
			//because yk, the text segment and the data segment are different the two stages will never ask for the same block....
			exit(1);
		}
	}

	log_info("block not found, looking if theres a space that can be freed..");

	for(int i = 0; i < n_way * block_size / 4; i += block_size / 4){
		cache_line * line;
		line = (cache + (index * n_way * block_size / 4) + i);
		if(line->valid == 0 || line->done == 1){
			log_debug("Theres atleast one space here... cool");
			return 0;
		}
	}
	
	log_info("okay theres no space here... structural hazard ! WAOW RARE");
	return -1;

}

uint32_t get_tag(uint32_t addr, int size, int block_size, int n_way){
	uint32_t block_offset = (uint32_t)log2((double)block_size);
	uint32_t n_sets = (uint32_t)(size/(block_size * n_way));
	uint32_t index_bits = (uint32_t)log2(n_sets);
	uint32_t index = (addr << (32 - index_bits - block_offset)) >> (32 - index_bits);
	uint32_t tag = addr >> (index_bits + block_offset);
	return tag;
}
