//MEMORY

extern uint64_t l1_d_hit;
extern uint64_t l1_d_miss;
extern uint64_t l1_i_hit;
extern uint64_t l1_i_miss;

extern uint64_t l2_hit;
extern uint64_t l2_miss;

extern uint64_t l3_hit;			
extern uint64_t l3_miss;

extern uint64_t n_reads;		//when the counter for a read is detected to be zero
extern uint64_t n_writes;		//when the counter for a write is detected to be zero
extern uint64_t n_instruction_reads;	//when the counter for a read_i is detected to be zero
extern uint64_t n_memory_stalls_i;	//whenever the mfc_i is checked and found not set
extern uint64_t n_memory_stalls_d;	//whenever the mfc is checked and found not set

//CPU

extern uint64_t branch_miss;		//in ma stage when the switch actually happens
extern uint64_t branch_hit;		//in ma stage when the switch actually happens
extern uint64_t n_branch_instructions;	//in ma stage when the switch check happens

extern uint64_t n_instructions;		//in fetch stage kya ?????
extern uint64_t cycles;

