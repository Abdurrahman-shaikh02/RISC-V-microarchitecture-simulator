//ALL STAT VARIABLES ARE MARKED WITH //stat COMMENT !!!
//
//MEMORY

extern uint64_t l1_d_hit_r;
extern uint64_t l1_d_hit_w;
extern uint64_t l1_d_miss_r;
extern uint64_t l1_d_miss_w;
//extern uint64_t l1_d_evictions;	//after the call to evict... after verifying that its not a structural hazard	(THESE WILL BE SAME AS THE NO OF MISSES !!!!)
extern uint64_t l1_d_writebacks;	//in the evict function because the writeback happens only in the evict function	eviction.c

extern uint64_t l1_i_hit_r;
extern uint64_t l1_i_miss_r;
//extern uint64_t l1_i_evictions;
extern uint64_t l1_i_writebacks;

extern uint64_t l2_hit_r;
extern uint64_t l2_hit_w;
extern uint64_t l2_miss_r;
extern uint64_t l2_miss_w;
//extern uint64_t l2_evictions;
extern uint64_t l2_writebacks;

extern uint64_t l3_hit_r;			
extern uint64_t l3_hit_w;
extern uint64_t l3_miss_r;
extern uint64_t l3_miss_w;
//extern uint64_t l3_evictions;
extern uint64_t l3_writebacks;

extern uint64_t n_reads;		//when the counter for a read is detected to be zero	memory.c
extern uint64_t n_writes;		//when the counter for a write is detected to be zero	memory.c
extern uint64_t n_instruction_reads;	//when the counter for a read_i is detected to be zero	memory.c
extern uint64_t n_read_i_stalls;	//when the counter is set	memory.c
extern uint64_t n_read_d_stalls;	//when the counter is set	memory.c
extern uint64_t n_write_d_stalls;	//when the counter is set	memory.c
extern uint64_t n_structural_hazards;	//whenever the set counter function returns -1	memory.c

//CPU

//Control hazards:
extern uint64_t branches_taken;		//in ma stage when the switch actually happens		ma.c
extern uint64_t branches_not_taken;	//in ma stage when the switch actually happens		ma.c
extern uint64_t branch_miss;		//in ma stage when the switch actually happens		ma.c
extern uint64_t branch_hit;		//in ma stage when the switch actually happens		ma.c
extern uint64_t n_branch_instructions;	//in ma stage when the switch check happens		ma.c
extern uint64_t id_flushes;		//(branch prediction flushes)in main where the flushes are handled	simulator.c
extern uint64_t ex_flushes;		//(branch/jump flushes)in main where the flushes are handled		simulator.c

//Data hazards:
extern uint64_t n_forwards_to_ex;	//every if else clause...	forwarding.c
extern uint64_t n_forwards_to_ma;	//every if else clause...	forwarding.c

extern uint64_t n_instructions;		//in fetch stage kya ?????
extern uint64_t cycle;

void display_stats(char * path);
