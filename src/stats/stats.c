#include "header.h"
#include "stats.h"

uint64_t l1_d_hit_r;
uint64_t l1_d_hit_w;
uint64_t l1_d_miss_r;
uint64_t l1_d_miss_w;

uint64_t l1_i_hit_r;
uint64_t l1_i_miss_r;

uint64_t l2_hit_r;
uint64_t l2_hit_w;
uint64_t l2_miss_r;
uint64_t l2_miss_w;

uint64_t l3_hit_r;			
uint64_t l3_hit_w;
uint64_t l3_miss_r;
uint64_t l3_miss_w;

uint64_t n_reads;		//when the counter for a read is detected to be zero	memory.c
uint64_t n_writes;		//when the counter for a write is detected to be zero	memory.c
uint64_t n_instruction_reads;	//when the counter for a read_i is detected to be zero	memory.c
uint64_t n_read_i_stalls;	//when the counter is set	memory.c
uint64_t n_read_d_stalls;	//when the counter is set	memory.c
uint64_t n_write_d_stalls;	//when the counter is set	memory.c
uint64_t n_structural_hazards;	//whenever the set counter function returns -1	memory.c

//CPU

uint64_t branch_miss;		//in ma stage when the switch actually happens		ma.c
uint64_t branch_hit;		//in ma stage when the switch actually happens		ma.c
uint64_t n_branch_instructions;	//in ma stage when the switch check happens		ma.c

uint64_t n_forwards_to_ex;	//every if else clause...
uint64_t n_forwards_to_ma;	//every if else clause...

//extern uint64_t n_instructions;		//in fetch stage kya ?????


void display_stats(char * path){

    FILE * fd = NULL;

    if(path == NULL){
	fd = stdout;
    }else{
	fd = fopen(path, "w");
    }


    if(fd == NULL){
	log_fatal("Couldnt open file for memory dump.");
	exit(1);
    }

    fprintf(fd, "\n==================== Simulation Statistics ====================\n");

    fprintf(fd, "\n-------------------- L1 Data Cache --------------------\n");
    fprintf(fd, "L1 D Read Hits        : %" PRIu64 "\n", l1_d_hit_r);
    fprintf(fd, "L1 D Write Hits       : %" PRIu64 "\n", l1_d_hit_w);
    fprintf(fd, "L1 D Read Misses      : %" PRIu64 "\n", l1_d_miss_r);
    fprintf(fd, "L1 D Write Misses     : %" PRIu64 "\n", l1_d_miss_w);

    fprintf(fd, "\n---------------- Instruction Cache --------------------\n");
    fprintf(fd, "L1 I Read Hits        : %" PRIu64 "\n", l1_i_hit_r);
    fprintf(fd, "L1 I Read Misses      : %" PRIu64 "\n", l1_i_miss_r);

    fprintf(fd, "\n---------------------- L2 Cache -----------------------\n");
    fprintf(fd, "L2 Read Hits          : %" PRIu64 "\n", l2_hit_r);
    fprintf(fd, "L2 Write Hits         : %" PRIu64 "\n", l2_hit_w);
    fprintf(fd, "L2 Read Misses        : %" PRIu64 "\n", l2_miss_r);
    fprintf(fd, "L2 Write Misses       : %" PRIu64 "\n", l2_miss_w);

    fprintf(fd, "\n---------------------- L3 Cache -----------------------\n");
    fprintf(fd, "L3 Read Hits          : %" PRIu64 "\n", l3_hit_r);
    fprintf(fd, "L3 Write Hits         : %" PRIu64 "\n", l3_hit_w);
    fprintf(fd, "L3 Read Misses        : %" PRIu64 "\n", l3_miss_r);
    fprintf(fd, "L3 Write Misses       : %" PRIu64 "\n", l3_miss_w);

    fprintf(fd, "\n---------------- Memory Requests ----------------------\n");
    fprintf(fd, "Data Reads            : %" PRIu64 "\n", n_reads);
    fprintf(fd, "Data Writes           : %" PRIu64 "\n", n_writes);
    fprintf(fd, "Instruction Reads     : %" PRIu64 "\n", n_instruction_reads);

    fprintf(fd, "\n--------------------- Stall Counts --------------------\n");
    fprintf(fd, "Instruction Read Stalls : %" PRIu64 "\n", n_read_i_stalls);
    fprintf(fd, "Data Read Stalls        : %" PRIu64 "\n", n_read_d_stalls);
    fprintf(fd, "Data Write Stalls       : %" PRIu64 "\n", n_write_d_stalls);
    fprintf(fd, "Structural Hazards      : %" PRIu64 "\n", n_structural_hazards);

    fprintf(fd, "\n-------------------- Branch Stats ---------------------\n");
    fprintf(fd, "Branch Instructions   : %" PRIu64 "\n", n_branch_instructions);
    fprintf(fd, "Branch Hits           : %" PRIu64 "\n", branch_hit);
    fprintf(fd, "Branch Misses         : %" PRIu64 "\n", branch_miss);


    fprintf(fd, "\n-------------------- Forwarding -----------------------\n");
    fprintf(fd, "Forwards to EX        : %" PRIu64 "\n", n_forwards_to_ex);
    fprintf(fd, "Forwards to MA        : %" PRIu64 "\n", n_forwards_to_ma);

    /*
       fprintf(fd, "\n---------------- CPU Statistics -----------------------\n");
       fprintf(fd, "Instructions Retired  : %" PRIu64 "\n", n_instructions);
       */

    fprintf(fd, "\n=======================================================\n");


    if(path != NULL){
	fclose(fd);
    }
}
