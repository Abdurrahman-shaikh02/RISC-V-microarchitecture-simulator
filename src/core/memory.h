extern uint32_t text_segment_limit;	//preferably something divisible by 4... :)
extern uint8_t mfc_i;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
extern uint32_t mar_i;
extern uint32_t mbr_i;

extern uint8_t mfc;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
extern uint32_t mar;
extern uint32_t mbr;

extern char ** instructions;
extern uint32_t total_number_of_instructions;
extern uint8_t * dram;

extern int STRUCTURAL_HAZARD_STALL;

void read_memory_i(int cancel);		//in case of a flush one must also cancel any pending memory instruction reads... non zero value of cancel resets the internal counter and sets the mfc_i

void read_memory(uint32_t opcode);	//need to implement aligned accesses only, REMEMBER THAT

void write_memory(uint32_t opcode);

void init_memory(char * instruction_file, char * data_file);		//this function needs to call both init i and init d
void init_memory_i(char * path);	//to load the program into the ram... 
void init_memory_d(char * path);

void display_memory();
void free_instructions(void);
void display_instructions();

extern FILE * memory_access_history_file_d;
extern FILE * memory_access_history_file_i;
#define l1_read_cycles 3
#define l1_read_i_cycles 3
#define l1_write_cycles 3

