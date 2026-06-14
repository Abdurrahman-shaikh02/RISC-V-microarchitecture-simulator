extern int cycle;
extern int REPLACEMENT_POLICY;
extern int CACHE_LEVELS;	//-1 means no 0 stalls for each mem operation, 0 means only ram, 1 means l1 and ram, 2 means l1 l2 and ram and so on
extern int WRITE_POLICY;
extern int WRITE_MISS_POLICY;

typedef struct cache_line{
	uint32_t data;
	uint32_t addr;
	uint8_t valid;
	uint8_t dirty;
	uint8_t done;	//are we done with the transaction related to this memory block ? OR are we allowed to evict this block ?
	uint64_t reference;
} cache_line;


extern cache_line * l1_i;		//dynamically allocated...
extern cache_line * l1_d;
extern cache_line * l2;
extern cache_line * l3;
//cache_line * ram;


//these are general functions... to be called only when the target BLOCK exists... -1 if block not found, 0 otherwise
int set_valid(uint32_t addr, int valid, cache_line * cache, int size, int block_size, int n_way);	//cache, size, block size, n-way set
int set_dirty(uint32_t addr, int dirty, cache_line * cache, int size, int block_size, int n_way);
int set_done(uint32_t addr, int done, cache_line * cache, int size, int block_size, int n_way);
int set_reference(uint32_t addr, uint64_t reference, cache_line * cache, int size, int block_size, int n_way);
//not making set addr and set data because thes are not common for the same block

uint32_t get_tag(uint32_t addr, int size, int block_size, int n_way);	//this is a general function that can calculate the tag using the cache specs...

int find_block(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);
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
int replace(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);		//a dispatch for different replacement policies
//if the block to be replaced is dirty and we're using writeback then give a larger stall
int replace_fifo(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);
int replace_lru(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);
int replace_mru(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);
int replace_lfu(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);

void copy_from_mem(uint32_t addr, cache_line * cache, int n_words);	//copy from ram to cache[0] onwards
//so you need to pass the pointer where the block starts

//this function assumes that the target block exists... verify that by find_block
void touch(cache_line * cache, int size, int block_size, int n_way);	//can implement it as a dispatch function
//call this after any kind of access to a block update the reference bits as per the chosen cache eviction policy

//READ MECHANISM
int set_read_counter();		//called by read; -2 if it needs a structural hazard stall
int set_read_i_counter();
//the set counter is supposed to find_blocks, set structural hazards if required... replace and make proper changes...everything...

//all of these are called in a recursive fashion(one calls the other if it is enabled)
//all return a certain number of stalls which will then eventually be returned in set counter
int l1_i_read();
int l1_d_read();
int l2_read();
int l3_read();
//-------------------------------------------------------------------------------------------------------------------------------------




//WRITE MECHANISM
int set_write_counter();

//depending upon the chosen policy.... do the thing and return stalls
int l1_i_write();
int l1_d_write();
int l2_write();
int l3_write();
//we wont be simulating full cahce behaviour, instead we just directyl write to memory always, and give a no of stalls based on latency and policy
//for write through we can add certain penalty... 
//### later on we can add a queue...(buffer) look below to find its plan
//for write back we can use the dirty bit... replace function gives higher stalls if the block to be evicted is dirty...
//if theres a write miss then write allocate/ no write allocate will return different stall counts

// WRITE-buffer plan:
// how to implement it... DO NOT CREATE ANY INCONSISTENCY BW THE CACHE AND THE MEMORY
// instead if it is not found, look in the buffer if yes then give a smaller stall(data will always be taken from ram directly)

//these will be dispatched to by the write function
//void write_through();
//void write_back();


//void load_from_ram(uint)
//void configure_memory();	//allocate the arrays... read specifications from config file.. also check for correctness of specs
