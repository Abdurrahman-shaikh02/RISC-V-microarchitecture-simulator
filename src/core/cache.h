extern int REPLACEMENT_POLICY;

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


//int set_read_counter();		//called by read; -2 if it needs a structural hazard stall
//the set counter is supposed to find_blocks, set structural hazards if required... replace and make proper changes...everything...


//we are always going to replace directly from ram, to the given cache...
void replace(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);		//a dispatch for different replacement policies
void replace_fifo(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);	//how do we store history ? make queues for each set ?
void replace_lru(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);	//how do we store history ? we will assume infinite history bits ?


//also need to make this a general function like the setters...
int find_block(uint32_t addr, cache_line * cache, int size, int block_size, int n_way);
// THIS method is only for probing any cache level for a particular address
// MUST CALL BEFORE REPLACING OR SETTER FUNCTIONS
			//calculate the index and access block, check tag, bits etc
			//0: not found can be replaced(if there is atleast one block that is done in the set...(handle which on to evict in replacement policies...)),
			//1: found... can read..., -1: NOT DONE! need to cause a STRUCTURAL HAZARD STALL
			//stall if required
			//then call replace if required after checking bits... 
			//then read


//these will be dispatched to by the write function
//void write_through();
//void write_back();


uint32_t get_tag(uint32_t addr, int size, int block_size, int n_way);	//this is a general function that can calculate the tag using the cache specs...
//void load_from_ram(uint)
//void configure_memory();	//allocate the arrays... read specifications from config file.. also check for correctness of specs
