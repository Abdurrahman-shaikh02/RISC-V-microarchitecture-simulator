extern uint32_t text_segment_limit;	//THIS IS THE LAST VALID ADDRESS FOR THE CODE SEGMENT (eg : 7ff... although unaligned... it holds the last byte of the code segment)
extern uint32_t rodata_segment_limit;

extern int DRAM_READ_LATENCY;
extern int DRAM_WRITE_LATENCY;
extern int DRAM_SIZE;

