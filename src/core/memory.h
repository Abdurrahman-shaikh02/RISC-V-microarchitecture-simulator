extern uint32_t text_segment_limit;	//preferably something divisible by 4... :)
extern uint8_t mfc_i;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
extern uint32_t mar_i;
extern uint32_t mbr_i;

extern uint8_t mfc;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
extern uint32_t mar;
extern uint32_t mbr;

void read_memory_i();

void read_memory();

void write_memory();

void init_memory(char * path);	//to load the program into the ram... 

#define l1_read_cycles 1
#define l1_write_cycles 1

