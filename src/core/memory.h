extern uint32_t text_segment_limit;	//preferably something divisible by 4... :)
extern uint8_t mfc_i;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
extern uint32_t mar_i;
extern uint32_t mbr_i;

extern uint8_t mfc;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
extern uint32_t mar;
extern uint32_t mbr;

extern char * instructions[];

extern int STRUCTURAL_HAZARD_STALL;

void read_memory_i();

void read_memory(uint32_t opcode);	//need to implement aligned accesses only, REMEMBER THAT

void write_memory(uint32_t opcode);

void init_memory(char * path);	//to load the program into the ram... 
void init_memory_i(char * path);	//to load the program into the ram... 

void display_memory();
void free_instructions(void);
void display_instructions();

#define l1_read_cycles 1
#define l1_write_cycles 1

