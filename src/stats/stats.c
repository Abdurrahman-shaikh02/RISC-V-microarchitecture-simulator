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


void display_stats(void)
{
    printf("\n==================== Simulation Statistics ====================\n");

    printf("\n-------------------- L1 Data Cache --------------------\n");
    printf("L1 D Read Hits        : %" PRIu64 "\n", l1_d_hit_r);
    printf("L1 D Write Hits       : %" PRIu64 "\n", l1_d_hit_w);
    printf("L1 D Read Misses      : %" PRIu64 "\n", l1_d_miss_r);
    printf("L1 D Write Misses     : %" PRIu64 "\n", l1_d_miss_w);

    printf("\n---------------- Instruction Cache --------------------\n");
    printf("L1 I Read Hits        : %" PRIu64 "\n", l1_i_hit_r);
    printf("L1 I Read Misses      : %" PRIu64 "\n", l1_i_miss_r);

    printf("\n---------------------- L2 Cache -----------------------\n");
    printf("L2 Read Hits          : %" PRIu64 "\n", l2_hit_r);
    printf("L2 Write Hits         : %" PRIu64 "\n", l2_hit_w);
    printf("L2 Read Misses        : %" PRIu64 "\n", l2_miss_r);
    printf("L2 Write Misses       : %" PRIu64 "\n", l2_miss_w);

    printf("\n---------------------- L3 Cache -----------------------\n");
    printf("L3 Read Hits          : %" PRIu64 "\n", l3_hit_r);
    printf("L3 Write Hits         : %" PRIu64 "\n", l3_hit_w);
    printf("L3 Read Misses        : %" PRIu64 "\n", l3_miss_r);
    printf("L3 Write Misses       : %" PRIu64 "\n", l3_miss_w);

    printf("\n---------------- Memory Requests ----------------------\n");
    printf("Data Reads            : %" PRIu64 "\n", n_reads);
    printf("Data Writes           : %" PRIu64 "\n", n_writes);
    printf("Instruction Reads     : %" PRIu64 "\n", n_instruction_reads);

    printf("\n--------------------- Stall Counts --------------------\n");
    printf("Instruction Read Stalls : %" PRIu64 "\n", n_read_i_stalls);
    printf("Data Read Stalls        : %" PRIu64 "\n", n_read_d_stalls);
    printf("Data Write Stalls       : %" PRIu64 "\n", n_write_d_stalls);
    printf("Structural Hazards      : %" PRIu64 "\n", n_structural_hazards);

    printf("\n-------------------- Branch Stats ---------------------\n");
    printf("Branch Instructions   : %" PRIu64 "\n", n_branch_instructions);
    printf("Branch Hits           : %" PRIu64 "\n", branch_hit);
    printf("Branch Misses         : %" PRIu64 "\n", branch_miss);


    printf("\n-------------------- Forwarding -----------------------\n");
    printf("Forwards to EX        : %" PRIu64 "\n", n_forwards_to_ex);
    printf("Forwards to MA        : %" PRIu64 "\n", n_forwards_to_ma);
    
    /*
    printf("\n---------------- CPU Statistics -----------------------\n");
    printf("Instructions Retired  : %" PRIu64 "\n", n_instructions);
    */

    printf("\n=======================================================\n");
}
