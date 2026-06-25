//--------------------------------------------------------------
//replacement polics: 0 is fifo , 1 if lru, 2 is .....



//-----------------------------------------------------------------
extern uint64_t cycle;		//global cycle
extern int CACHE_LEVELS;	//-1 means 0 stalls for each mem operation, 0 means only ram, 1 means l1 and ram, 2 means l1 l2 and ram and so on


typedef struct cache_line{
	uint32_t data;
	uint32_t addr;
	uint8_t valid;
	uint8_t dirty;
	uint8_t done;	//are we done with the transaction related to this memory block ? OR are we allowed to evict this block ?
	uint64_t reference;
} cache_line;


typedef struct cache_level{
	int level;
	cache_line * cache;	//allocate this dynamically
	int write_latency;
	int read_latency;
	int cache_size;
	int block_size;
	int associativity;	//0 is fully associative
	int write_policy;	//0 is write through, 1 is write back
	int write_miss_policy;	//0 is no write allocate, 1 write allocate
	int replacement_policy;	//0 is fifo, 1 is lru
} cache_level;

extern cache_level l1_i;
extern cache_level l1_d;
extern cache_level l2;
extern cache_level l3;

extern int DRAM_READ_LATENCY;
extern int DRAM_WRITE_LATENCY;
extern int DRAM_SIZE;


//these are general functions... to be called only when the target BLOCK exists... -1 if block not found, 0 otherwise
int set_valid(uint32_t addr, int valid, cache_level cache);	//cache, size, block size, n-way set
int set_dirty(uint32_t addr, int dirty, cache_level cache);
int set_done(uint32_t addr, int done, cache_level cache);
int set_reference(uint32_t addr, uint64_t reference, cache_level cache);
//not making set addr and set data because thes are not common for the same block

uint32_t get_tag(uint32_t addr, int size, int block_size, int n_way);	//this is a general function that can calculate the tag using the cache specs...

int find_block(uint32_t addr, cache_level cache);
// THIS method is only for probing any cache level for a particular address
// this doesnt distinguish bw a dirty block and a non dirty block so write back needs seperate checking
// MUST CALL BEFORE REPLACING OR SETTER FUNCTIONS
			//calculate the index and access block, check tag, bits etc
			//0: not found can be replaced(if there is atleast one block that is done in the set...(handle which on to evict in replacement policies...)),
			//1: found... can read..., -1: NOT DONE! need to cause a STRUCTURAL HAZARD STALL
			//stall if required
			//then call replace if required after checking bits... 
			//then read


//we are always going to replace directly from ram, to the given cache...
//the following functions assume that atleast one space exists to be replaced...
//these are general functions and must be called using the recommended guidlines....
int evict(uint32_t addr, cache_level cache);		//a dispatch for different replacement policies
//evict is to only return the stall cause due to write back thats it!
//if the block to be replaced is dirty and we're using writeback then give a larger stall
int evict_fifo(uint32_t addr, cache_level cache);
int evict_lru(uint32_t addr, cache_level cache);
int evict_mru(uint32_t addr, cache_level cache);
int evict_lfu(uint32_t addr, cache_level cache);

void copy_from_mem(uint32_t addr, cache_line * cache, int n_words);	//copy from ram to cache[0] onwards
//so you need to pass the pointer where the block starts

//this function assumes that the target block exists... verify that by find_block
void touch(uint32_t addr, cache_level cache);	//can implement it as a dispatch function
//call this after any kind of access to a block update the reference bits as per the chosen cache eviction policy

void complete_below(uint32_t addr, cache_level s);	//set done = 1 for s and all level below it


//READ MECHANISM
int set_read_counter(uint32_t addr);		//called by read; -2 if it needs a structural hazard stall
void complete_read(uint32_t addr);
int set_read_i_counter(uint32_t addr);
void complete_read_i(uint32_t addr);
//the set counter is supposed to find_blocks, set structural hazards if required... replace and make proper changes...everything...

//all of these are called in a recursive fashion(one calls the other if it is enabled)
//all return a certain number of stalls which will then eventually be returned in set counter
int l1_i_read(uint32_t addr);
int l1_d_read(uint32_t addr);
int l2_read(uint32_t addr);
int l3_read(uint32_t addr);
//-------------------------------------------------------------------------------------------------------------------------------------




//WRITE MECHANISM
int set_write_counter(uint32_t addr);
void complete_write(uint32_t addr);

//depending upon the chosen policy.... do the thing and return stalls
int l1_d_write(uint32_t addr);
int l2_write(uint32_t addr);
int l3_write(uint32_t addr);

//we wont be simulating full cache behaviour, instead we just directyl write to memory always, and give a no of stalls based on latency and policy
//for write through we can add certain penalty... 
//### later on we can add a queue...(buffer) look below to find its plan
//for write back we can use the dirty bit... replace function gives higher stalls if the block to be evicted is dirty...
//if theres a write miss then write allocate/ no write allocate will return different stall counts

// WRITE-buffer plan:(later)
// how to implement it... DO NOT CREATE ANY INCONSISTENCY BW THE CACHE AND THE MEMORY
// instead if it is not found, look in the buffer if yes then give a smaller stall(data will always be taken from ram directly)

