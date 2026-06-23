#include "header.h"
#include "cache.h"
#include "memory.h"

void complete_below(uint32_t addr, cache_level s){
	log_info("Completing below operation");
	switch(s.level){
		case 1:
			if(CACHE_LEVELS > 0){
				set_done(addr, 1, s);
			}
		case 2:
			if(CACHE_LEVELS > 1){
				set_done(addr, 1, l2);
			}
		case 3:
			if(CACHE_LEVELS > 2){
				set_done(addr, 1, l3);
			}
			break;
		default:
			log_fatal("Illegal cache level");
			exit(1);
	}
}


void touch(uint32_t addr, cache_level s){
	//touch only on cache hits
	if(s.replacement_policy == 1){
		int set = set_reference(addr, cycle, s);
		if(set != 0){
			log_fatal("touch failed...");
			exit(1);
		}
	}
}


int find_block(uint32_t addr, cache_level s){
	cache_line * cache = s.cache;
	int size = s.cache_size;
	int n_way = s.associativity;
	int block_size = s.block_size;

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
		if(line->valid == 1 && line_tag == tag && line->done == 1){
			log_info("cache hit !");
			return 1;
		}else if(line->valid == 1 && line_tag == tag && line->done == 0){
			//structural hazard
			log_fatal("structural hazard waow thats rare... actually impossible... somethings wrong bud !");
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


void configure_cache(){
	l1_d.cache = (cache_line *)malloc(2 * 1024 / 4 * sizeof(cache_line));	//2 kb l1_d ram
	l1_i.cache = (cache_line *)malloc(2 * 1024 / 4 * sizeof(cache_line));	//2 kb l1_i ram
	l2.cache = (cache_line *)malloc(8 * 1024 / 4 * sizeof(cache_line));	//8 kb l2 ram
	l3.cache = (cache_line *)malloc(16 * 1024 / 4 * sizeof(cache_line));	//16 kb l3 ram
	
	l1_d.cache_size = 1024 * 2;	//2k
	l1_d.block_size = 16;
	l1_d.associativity = 2;
	l1_d.read_latency = 1;
	l1_d.write_latency = 1;
	l1_d.write_miss_policy = 0;
	l1_d.replacement_policy = 0;
	l1_d.write_policy = 1;
	l1_d.level = 1;

	l1_i.cache_size = 1024 * 2;	//2k
	l1_i.block_size = 16;
	l1_i.associativity = 2;
	l1_i.read_latency = 1;
	l1_i.write_latency = 1;
	l1_i.write_miss_policy = 0;
	l1_i.replacement_policy = 0;
	l1_i.write_policy = 1;
	l1_i.level = 1;


	l2.cache_size = 1024 * 8;	//8k
	l2.block_size = 32;
	l2.associativity = 4;
	l2.read_latency = 2;
	l2.write_latency = 2;
	l2.write_miss_policy = 0;
	l2.replacement_policy = 0;
	l2.write_policy = 1;
	l2.level = 2;

	l3.cache_size = 1024 * 16;	//16k
	l3.block_size = 64;
	l3.associativity = 8;
	l3.read_latency = 3;
	l3.write_latency = 3;
	l3.write_miss_policy = 0;
	l3.replacement_policy = 0;
	l3.write_policy = 1;
	l3.level = 3;

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

	DRAM_READ_LATENCY = 4;
	DRAM_WRITE_LATENCY = 4;
	DRAM_SIZE = 32 * 1024;

	log_info("configured...");
}
