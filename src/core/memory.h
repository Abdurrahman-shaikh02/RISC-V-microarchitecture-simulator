#include <stdint.h>

static uint32_t text_segment_limit = 0x1000;	//preferably something divisible by 4... :)
static uint8_t mfc_i = 1;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
static uint32_t mar_i;
static uint32_t mbr_i;

static uint8_t mfc = 1;	    //0 means not completed	make sure to call read/write ONLY if mfc is 1
static uint32_t mar;
static uint32_t mbr;

void read_memory_i();

void read_memory();

void write_memory();

void init_memory(char * path);	//to load the program into the ram... 

#define l1_read_cycles 1
#define l1_write_cycles 1

