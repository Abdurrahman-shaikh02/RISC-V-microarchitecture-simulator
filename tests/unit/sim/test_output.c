// src/stats/stats.c and src/utils/utility.c: the exact text of the statistics
// report and of the register dumps (part of the user interface)
#include "sim_support.h"

TEST(stats_report_format){
	uint64_t * counters[] = {
		&l1_d_hit_r, &l1_d_hit_w, &l1_d_miss_r, &l1_d_miss_w, &l1_d_writebacks,
		&l1_i_hit_r, &l1_i_miss_r, &l1_i_writebacks,
		&l2_hit_r, &l2_hit_w, &l2_miss_r, &l2_miss_w, &l2_writebacks,
		&l3_hit_r, &l3_hit_w, &l3_miss_r, &l3_miss_w, &l3_writebacks,
		&n_reads, &n_writes, &n_instruction_reads,
		&n_read_i_stalls, &n_read_d_stalls, &n_write_d_stalls, &n_structural_hazards,
		&n_branch_instructions, &branches_taken, &branches_not_taken, &branch_hit, &branch_miss,
		&id_flushes, &ex_flushes, &n_forwards_to_ex, &n_forwards_to_ma, &n_instructions, &cycle,
	};
	for(unsigned i = 0; i < sizeof(counters) / sizeof(counters[0]); i++) *counters[i] = i + 1;
	display_stats("stats.txt");
	CHECK_STR_EQ(test_read_file("stats.txt"),
		"\n==================== Simulation Statistics ====================\n"
		"\n-------------------- L1 Data Cache --------------------\n"
		"L1 D Read Hits        : 1\n"
		"L1 D Write Hits       : 2\n"
		"L1 D Read Misses      : 3\n"
		"L1 D Write Misses     : 4\n"
		"L1 D WriteBacks       : 5\n"
		"\n---------------- Instruction Cache --------------------\n"
		"L1 I Read Hits        : 6\n"
		"L1 I Read Misses      : 7\n"
		"L1 I WriteBacks       : 8\n"
		"\n---------------------- L2 Cache -----------------------\n"
		"L2 Read Hits          : 9\n"
		"L2 Write Hits         : 10\n"
		"L2 Read Misses        : 11\n"
		"L2 Write Misses       : 12\n"
		"L2 WriteBacks         : 13\n"
		"\n---------------------- L3 Cache -----------------------\n"
		"L3 Read Hits          : 14\n"
		"L3 Write Hits         : 15\n"
		"L3 Read Misses        : 16\n"
		"L3 Write Misses       : 17\n"
		"L3 WriteBacks         : 18\n"
		"\n---------------- Memory Requests ----------------------\n"
		"Data Reads            : 19\n"
		"Data Writes           : 20\n"
		"Instruction Reads     : 21\n"
		"\n--------------------- Stall Counts --------------------\n"
		"Instruction Read Stalls : 22\n"
		"Data Read Stalls        : 23\n"
		"Data Write Stalls       : 24\n"
		"Structural Hazards      : 25\n"
		"\n-------------------- Branch Stats ---------------------\n"
		"Branch Instructions   : 26\n"
		"Branches Taken        : 27\n"
		"Branches Not Taken    : 28\n"
		"Branch Hits           : 29\n"
		"Branch Misses         : 30\n"
		"\n-------------------- Flushes -----------------------\n"
		"Flushes from ID       : 31\n"
		"Flushes from EX       : 32\n"
		"\n-------------------- Forwarding -----------------------\n"
		"Forwards to EX        : 33\n"
		"Forwards to MA        : 34\n"
		"\n---------------- CPU Statistics -----------------------\n"
		"Instructions Retired  : 35\n"
		"Total Cycles  : 36\n"
		"\n=======================================================\n");
}

TEST(stats_report_to_stdout){
	cycle = 42;
	char out[8192];
	int st;
	RUN_ISOLATED(out, st, display_stats(NULL));
	CHECK_EQ(st, 0);
	CHECK_CONTAINS(out, "Total Cycles  : 42\n");
}

TEST(register_dump_format){
	sim_setup(-1);
	for(int i = 1; i < 32; i++) reg_file[i] = 0x01010101u * i;
	reg_file[31] = 0xFFFFFFFF;
	display_general_purpose_registers("regs");
	char * s = test_read_file("regs");
	CHECK(s != NULL);
	if(!s) return;
	CHECK(strncmp(s, "r00 : 00000000\nr01 : 01010101\nr02 : 02020202\n", 45) == 0);
	CHECK_CONTAINS(s, "r10 : 0a0a0a0a\n");
	CHECK_CONTAINS(s, "r30 : 1e1e1e1e\nr31 : ffffffff\n");
	CHECK_EQ(strlen(s), 32 * 15);
}

TEST(internal_register_dump_format){
	sim_setup(-1);
	pc = 0x1234;
	ir = 0xabcdef01;
	mar = 0x40000;
	mbr = 0x55;
	mar_i = 16;
	mbr_i = 19;
	if_id = (IF_ID){0x13, 0x10, 0x14, BLANK_INS};
	id_ex.R1 = 0x77;
	id_ex.cs_ex.type = 3;
	id_ex.cs_ma.read_write_opcode = 0xa;
	ex_ma.result = 0x88;
	ex_ma.cs_wb.wb = 1;
	ma_wb.nrd = 5;
	display_internal_registers("iregs");
	char * s = test_read_file("iregs");
	CHECK(s != NULL);
	if(!s) return;
	CHECK(strncmp(s, "========================================\nCORE REGISTERS\n========================================\n"
		"pc       : 00001234\nir       : abcdef01\n", 107) == 0);
	CHECK_CONTAINS(s, "mar      : 00040000\nmbr      : 00000055\nmar_i    : 16\nmbr_i    : 19\n");	// mar_i/mbr_i in decimal
	CHECK_CONTAINS(s, "IF_ID:\nIR       : 00000013\nPC       : 00000010\nPC_next  : 00000014\n");
	CHECK_CONTAINS(s, "R1       : 00000077\n");
	CHECK_CONTAINS(s, "type                : 03\n");
	CHECK_CONTAINS(s, "read_write_opcode   : 0a\n");
	CHECK_CONTAINS(s, "EX_MA:\nPC_next  : 00000000\nR2       : 00000000\nresult   : 00000088\n");
	CHECK_CONTAINS(s, "MA_WB:\nresult   : 00000000\nnrd      : 00000005\n");
	size_t len = strlen(s);
	CHECK(len > 40 && strcmp(s + len - 41, "========================================\n") == 0);
}

TEST(logger_writes_only_at_or_above_level){
	CURRENT_LEVEL = LOG_WARN;
	log_info("hidden");
	log_warn("shown warning");
	log_error("shown error");
	char * s = test_read_file("log.log");
	CHECK(s != NULL);
	if(!s) return;
	CHECK_CONTAINS(s, "Date and Time : ");
	CHECK_CONTAINS(s, "[WARN]: shown warning\n[ERROR]: shown error\n");
	CHECK(strstr(s, "hidden") == NULL);
}

TEST(logger_quiet_level_creates_no_file){
	sim_quiet();
	log_error("x");
	log_msg(LOG_FATAL, "y");
	CHECK(access("log.log", F_OK) != 0);
}

TEST_SUITE("output",
	T(stats_report_format),
	T(stats_report_to_stdout),
	T(register_dump_format),
	T(internal_register_dump_format),
	T(logger_writes_only_at_or_above_level),
	T(logger_quiet_level_creates_no_file),
)
