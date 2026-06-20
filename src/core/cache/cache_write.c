#include "header.h"
#include "cache.h"
#include "memory.h"

void complete_write(uint32_t addr){
	log_info("completing write");
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

int set_write_counter(uint32_t addr){
	log_info("Data write");

	if(CACHE_LEVELS == -1){
		log_info("Ideal memory setup... no stalls");
		return 0;
	}

	if(CACHE_LEVELS > 0){
		int stall_count = l1_d_write(addr);

		if(stall_count == -1){
			//structural hazard
			log_info("Structural hazard... need to stall");
			return -1;
		}

		return stall_count;
	}else{
		log_info("There is no cache... so writing directly into memory");
		return DRAM_WRITE_LATENCY;
	}

}

int l1_d_write(uint32_t addr){
	//this is called only if CACHE_LEVEL > 0
	
	//lookup
	int found = find_block(addr, l1_d);
	//handle structural hazard only if is write allocate...
	

	if(found == 1){
		//hit
		log_info("Write hit at l1_d");
		if(l1_d.write_policy == 0){
			//write through
			if(CACHE_LEVELS > 1){
				//next level
				int stall_count = l2_write(addr);

				if(stall_count == -1){
					log_info("Structural hazard... need to stall");
					return -1;
				}

				touch(addr, l1_d);
				set_done(addr, 0, l1_d);

				return l1_d.write_latency + stall_count;
			}else{
				//write directly to memory
				touch(addr, l1_d);
				set_done(addr, 0, l1_d);

				return l1_d.write_latency + DRAM_WRITE_LATENCY;
			}
		}else if(l1_d.write_policy == 1){
			//write back
			set_dirty(addr, 1, l1_d);

			touch(addr, l1_d);
			set_done(addr, 0, l1_d);

			return l1_d.write_latency;
		}
		log_fatal("Illegal write policy at l1_d");
		exit(1);
	}else{
		//write miss
		log_info("Write miss at l1_d");

		if(found == -1 && l1_d.write_miss_policy == 1){
			//if write allocate
			log_info("Structural hazard at l1_d write");
			return -1;
		}

		if(l1_d.write_miss_policy == 1){
			//write allocate
			log_info("Write allocate policy so issuing a read operation");

			int stall_count = l1_d_read(addr);
			if(stall_count == -1){
				log_info("Structural hazard... need to stall");
				return -1;
			}
			complete_below(addr, l1_d);
			
			log_info("Calling l1_d write again... this time it will be a write hit");
			int stall_count2 = l1_d_write(addr);
			if(stall_count2 == -1){
				log_fatal("Structural Hazard at l1_d! Shouldnt happen at all somethings wrong !");
				exit(1);
			}

			return stall_count + stall_count2;

		}else if(l1_d.write_miss_policy == 0){
			//no write allocate
			log_info("No write allocate so issuing write to the lower level");

			if(CACHE_LEVELS > 1){
				//next level is l2
				int stall_count = l2_write(addr);

				if(stall_count == -1){
					log_info("Structural hazard... need to stall");
					return -1;
				}

				return stall_count;	//fully bypassed l1_d
			}else{
				//write directly to memory
				return DRAM_WRITE_LATENCY;	//fully bypassed l1_d
			}
		}else{
			log_fatal("Illegal write miss policy.");
			exit(1);
		}
	}
	
}

int l2_write(uint32_t addr){
	//this is called only if CACHE_LEVEL > 1
	
	//lookup
	int found = find_block(addr, l2);
	//handle structural hazard only if is write allocate...
	

	if(found == 1){
		//hit
		log_info("Write hit at l2");
		if(l2.write_policy == 0){
			//write through
			if(CACHE_LEVELS > 2){
				//next level
				int stall_count = l3_write(addr);

				if(stall_count == -1){
					log_info("Structural hazard... need to stall");
					return -1;
				}

				touch(addr, l2);
				set_done(addr, 0, l2);

				return l2.write_latency + stall_count;
			}else{
				//write directly to memory
				touch(addr, l2);
				set_done(addr, 0, l2);

				return l2.write_latency + DRAM_WRITE_LATENCY;
			}
		}else if(l2.write_policy == 1){
			//write back
			set_dirty(addr, 1, l2);

			touch(addr, l2);
			set_done(addr, 0, l2);

			return l2.write_latency;
		}
		log_fatal("Illegal write policy at l2");
		exit(1);
	}else{
		//write miss
		log_info("Write miss at l2");

		if(found == -1 && l2.write_miss_policy == 1){
			//if write allocate
			log_info("Structural hazard at l2 write");
			return -1;
		}

		if(l2.write_miss_policy == 1){
			//write allocate
			log_info("Write allocate policy so issuing a read operation");

			int stall_count = l2_read(addr);
			if(stall_count == -1){
				log_info("Structural hazard... need to stall");
				return -1;
			}
			complete_below(addr, l2);
			
			log_info("Calling l2 write again... this time it will be a write hit");
			int stall_count2 = l2_write(addr);
			if(stall_count2 == -1){
				log_fatal("Structural Hazard at l2! Shouldnt happen at all somethings wrong !");
				exit(1);
			}

			return stall_count + stall_count2;

		}else if(l2.write_miss_policy == 0){
			//no write allocate
			log_info("No write allocate so issuing write to the lower level");

			if(CACHE_LEVELS > 2){
				//next level is l3
				int stall_count = l3_write(addr);

				if(stall_count == -1){
					log_info("Structural hazard... need to stall");
					return -1;
				}

				return stall_count;	//fully bypassed l2
			}else{
				//write directly to memory
				return DRAM_WRITE_LATENCY;	//fully bypassed l2
			}
		}else{
			log_fatal("Illegal write miss policy.");
			exit(1);
		}
	}
	
}

int l3_write(uint32_t addr){
	//this is called only if CACHE_LEVEL > 2
	
	//lookup
	int found = find_block(addr, l3);
	//handle structural hazard only if is write allocate...
	

	if(found == 1){
		//hit
		log_info("Write hit at l3");
		if(l3.write_policy == 0){
			//write through
			//write directly to memory
			touch(addr, l3);
			set_done(addr, 0, l3);

			return l3.write_latency + DRAM_WRITE_LATENCY;
		}else if(l3.write_policy == 1){
			//write back
			set_dirty(addr, 1, l3);

			touch(addr, l3);
			set_done(addr, 0, l3);

			return l3.write_latency;
		}
		log_fatal("Illegal write policy at l3");
		exit(1);
	}else{
		//write miss
		log_info("Write miss at l3");

		if(found == -1 && l3.write_miss_policy == 1){
			//if write allocate
			log_info("Structural hazard at l3 write");
			return -1;
		}

		if(l3.write_miss_policy == 1){
			//write allocate
			log_info("Write allocate policy so issuing a read operation");

			int stall_count = l3_read(addr);
			if(stall_count == -1){
				log_info("Structural hazard... need to stall");
				return -1;
			}
			complete_below(addr, l3);
			
			log_info("Calling l3 write again... this time it will be a write hit");
			int stall_count2 = l3_write(addr);
			if(stall_count2 == -1){
				log_fatal("Structural Hazard at l3! Shouldnt happen at all somethings wrong !");
				exit(1);
			}

			return stall_count + stall_count2;

		}else if(l3.write_miss_policy == 0){
			//no write allocate
			log_info("No write allocate so issuing write to the lower level");
			//write directly to memory
			return DRAM_WRITE_LATENCY;	//fully bypassed l3
		}else{
			log_fatal("Illegal write miss policy.");
			exit(1);
		}
	}
	
}
