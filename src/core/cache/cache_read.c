#include "header.h"
#include "cache.h"
#include "memory.h"

//complete reads only set the done bits in every level...

void complete_read_i(uint32_t addr){
	log_info("completing instruction read");
	if(CACHE_LEVELS > 0){
		set_done(addr, 1, l1_i);
	}
	if(CACHE_LEVELS > 1){
		set_done(addr, 1, l2);
	}
	if(CACHE_LEVELS > 2){
		set_done(addr, 1, l3);
	}
}

void complete_read(uint32_t addr){
	log_info("completing data read");
	if(CACHE_LEVELS > 0){
		set_done(addr, 1, l1_d);
	}
	if(CACHE_LEVELS > 1){
		set_done(addr, 1, l2);
	}
	if(CACHE_LEVELS > 2){
		set_done(addr, 1, l3);
	}
}

int set_read_i_counter(uint32_t addr){
	log_info("Instruction read");

	if(CACHE_LEVELS == -1){
		log_info("Ideal memory setup... no stalls");
		return 0;
	}

	if(CACHE_LEVELS > 0){
		int stall_count = l1_i_read(addr);

		if(stall_count == -1){
			//structural hazard
			log_info("Structural hazard... need to stall");
			return -1;
		}

		return stall_count;
	}else{
		log_info("There is no cache... so looking directly into memory");
		return DRAM_READ_LATENCY;
	}
}

int set_read_counter(uint32_t addr){
	log_info("Data read");

	if(CACHE_LEVELS == -1){
		log_info("Ideal memory setup... no stalls");
		return 0;
	}

	if(CACHE_LEVELS > 0){
		int stall_count = l1_d_read(addr);

		if(stall_count == -1){
			//structural hazard
			log_info("Structural hazard... need to stall");
			return -1;
		}

		return stall_count;
	}else{
		log_info("There is no cache... so looking directly into memory");
		return DRAM_READ_LATENCY;
	}
}


int l1_i_read(uint32_t addr){
	//will be called only if CACHE_LEVELS > 0
	log_info("looking into l1_i cache");

	//lookup
	int found = find_block(addr, l1_i);

	//if found reset done but and return l1 read latency
	if(found == 1){
		log_info("Cache hit at l1_i");
		//touch only the level that was accessed, the lower levels do not need to know that an upper level had a cache hit on a line that they also contain
		touch(addr, l1_i);
		return l1_i.read_latency;
	}else if(found == -1){
		//theres a structural hazard here....
		log_warn("Yea a structural hazard in l1_i read isnt supposed to happen... somethings wrong! UNLESS we are not using split cache");
		return -1;
	}

	//if not then continue below...
	//reach here if theres an l1_i miss and theres a space available to be replaced...
	int stall_count = 0;
	if(CACHE_LEVELS > 1){
		//call l2 read, add up its return value and l1 read latency and return that...
		log_info("l1_i read miss...");
		stall_count = l2_read(addr);

		//see if a structural hazard has occured
		if(stall_count == -1){
			log_info("Structural hazard... need to make a stall");
			return -1;
		}
	
		//we know theres atleast one space in the cache
		stall_count += evict(addr, l1_i);
		
		//return stall + latency because it is taken from l2 cache(conceptually...)
		//(doesnt work that way in our implementation, we always copy form mem)
		return l1_i.read_latency + stall_count;
	}else{
		//if theres only one level of cache
		
		//we know theres atleast one space in the cache
		log_info("l1_i read miss...looking into memory now");
		stall_count = evict(addr, l1_i);

		//since theres only one level of cache... we have to take it form the ram...
		return l1_i.read_latency + DRAM_READ_LATENCY + stall_count;
	}
}


int l1_d_read(uint32_t addr){
	//will be called only if CACHE_LEVELS > 0
	log_info("looking into l1_d cache");

	//lookup
	int found = find_block(addr, l1_d);

	//if found reset done but and return l1 read latency
	if(found == 1){
		log_info("Cache hit at l1_d");
		//touch only the level that was accessed, the lower levels do not need to know that an upper level had a cache hit on a line that they also contain
		touch(addr, l1_d);
		return l1_d.read_latency;
	}else if(found == -1){
		//theres a structural hazard here....
		log_warn("Yea a structural hazard in l1_d read isnt supposed to happen... somethings wrong! UNLESS we are not using split cache");
		return -1;
	}
	
	int stall_count = 0;
	//if not then continue below...
	//reach here if theres an l1_d miss and theres a space available to be replaced...
	if(CACHE_LEVELS > 1){
		//call l2 read, add up its return value and l1 read latency and return that...
		log_info("l1_d read miss...");
		stall_count = l2_read(addr);

		//see if a structural hazard has occured
		if(stall_count == -1){
			log_info("Structural hazard... need to make a stall");
			return -1;
		}
	
		//we know theres atleast one space in the cache
		stall_count += evict(addr, l1_d);
		
		//return stall + latency because it is taken from l2 cache(conceptually...)
		//(doesnt work that way in our implementation, we always copy form mem)
		return l1_d.read_latency + stall_count;
	}else{
		//if theres only one level of cache
		
		//we know theres atleast one space in the cache
		log_info("l1_d read miss...looking into memory now");
		stall_count = evict(addr, l1_d);

		//since theres only one level of cache... we have to take it form the ram...
		return l1_d.read_latency + DRAM_READ_LATENCY + stall_count;
	}
}

int l2_read(uint32_t addr){
	//this will be called only if CACHE_LEVELS > 1
	//first lookup
	log_info("looking into l2 for read");
	int found = find_block(addr, l2);

	//if hit , return l2 latency...
	if(found == 1){
		log_info("Cache hit at l2");
		//touch only the level that was accessed, the lower levels do not need to know that an upper level had a cache hit on a line that they also contain
		touch(addr, l2);
		return l2.read_latency;
	}else if(found == -1){
		//structural hazard
		log_info("Structural hazard at l2");
		return -1;
	}
	
	int stall_count = 0;
	//if not found continue further...
	//if CACHE_LEVELS > 2 call l3 read ...then check if structural hazard... then evict, and then after that return the stall count + l2 latency...
	if(CACHE_LEVELS > 2){
		log_info("l2 read miss...");
		int stall_count = l3_read(addr);

		if(stall_count == -1){
			log_info("Structural hazard... need to make a stall");
			return -1;
		}

		//we know theres a free block to replace...
		stall_count += evict(addr, l2);

		return stall_count + l2.read_latency;
	}else{
		log_info("l2 read miss...looking into memory now");
		//if CACHE_LEVELS not > 2 then just evict and return l2 latency plus dram latency
		
		//we know a free space exists
		stall_count = evict(addr, l2);

		return l2.read_latency + DRAM_READ_LATENCY + stall_count;
	}
}


int l3_read(uint32_t addr){
	//called only if CACHE_LEVELS > 2
	
	//first lookup
	log_info("looking into l3 for read");
	int found = find_block(addr, l3);

	//if found cache hit return l3 read latency
	if(found == 1){
		log_info("Cache hit at l3");
		//touch only the level that was accessed, the lower levels do not need to know that an upper level had a cache hit on a line that they also contain
		touch(addr, l3);
		return l3.read_latency;
	}else if(found == -1){
		//structural hazard
		log_info("Structural hazard at l3");
		return -1;
	}
	
	int stall_count = 0;
	//if not found and not a structural hazard... then load from ram
	log_info("l3 read miss... looking into memory now");

	//we know thers atleast one free space..
	stall_count = evict(addr, l3);

	return l3.read_latency + DRAM_READ_LATENCY;
}





