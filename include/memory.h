extern uint32_t text_segment_limit;	//THIS IS THE LAST VALID ADDRESS FOR THE CODE SEGMENT (eg : 7ff... although unaligned... it holds the last byte of the code segment)
extern uint32_t rodata_segment_limit;
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

void open_trace_files(char * instruction_trace, char * data_trace);
void display_memory(char * path);
void free_instructions(void);
void display_instructions();

extern int trace;
extern FILE * memory_access_history_file_d;
extern FILE * memory_access_history_file_i;

