// src/memory/memory.c: memory ports, latency counters, traces and dumps
#include "sim_support.h"

static int calls_until_read_i_completes(uint32_t addr){
	mar_i = addr;
	int calls = 0;
	do{
		read_memory_i(0);
		calls++;
	}while(mfc_i == NOT_COMPLETED && calls < 10000);
	return calls;
}

static int calls_until_read_completes(uint32_t addr, uint32_t opcode){
	mar = addr;
	int calls = 0;
	do{
		read_memory(opcode);
		calls++;
	}while(mfc == NOT_COMPLETED && calls < 10000);
	return calls;
}

static int calls_until_write_completes(uint32_t addr, uint32_t opcode, uint32_t data){
	mar = addr;
	mbr = data;
	int calls = 0;
	do{
		write_memory(opcode);
		calls++;
	}while(mfc == NOT_COMPLETED && calls < 10000);
	return calls;
}

TEST(ideal_memory_completes_in_one_call){
	sim_setup(-1);
	sim_dram_set_word(0x40000, 0xDEADBEEF);
	sim_dram_set_word(0x10, 0x00000013);
	CHECK_EQ(calls_until_read_i_completes(0x10), 1);
	CHECK_EQ(mbr_i, 0x00000013);
	CHECK_EQ(calls_until_read_completes(0x40000, 0b010), 1);
	CHECK_EQ(mbr, 0xDEADBEEF);
	CHECK_EQ(calls_until_write_completes(0x40004, 0b010, 7), 1);
	CHECK_EQ(sim_dram_word(0x40004), 7);
	CHECK_EQ(n_read_i_stalls + n_read_d_stalls + n_write_d_stalls, 0);
}

TEST(dram_latency_is_the_number_of_calls){
	sim_setup(0);
	DRAM_READ_LATENCY = 5;
	DRAM_WRITE_LATENCY = 7;
	CHECK_EQ(calls_until_read_i_completes(0), 5);
	CHECK_EQ(calls_until_read_completes(0x40000, 0b010), 5);
	CHECK_EQ(calls_until_write_completes(0x40000, 0b010, 1), 7);
	CHECK_EQ(n_read_i_stalls, 4);
	CHECK_EQ(n_read_d_stalls, 4);
	CHECK_EQ(n_write_d_stalls, 6);
	CHECK_EQ(n_instruction_reads, 1);
	CHECK_EQ(n_reads, 1);
	CHECK_EQ(n_writes, 1);
}

TEST(write_is_only_visible_after_completion){
	sim_setup(0);
	DRAM_WRITE_LATENCY = 3;
	mar = 0x40000;
	mbr = 0x1234;
	write_memory(0b010);
	write_memory(0b010);
	CHECK_EQ(sim_dram_word(0x40000), 0);
	write_memory(0b010);
	CHECK_EQ(sim_dram_word(0x40000), 0x1234);
}

TEST(cancel_resets_instruction_read){
	sim_setup(0);
	DRAM_READ_LATENCY = 5;
	sim_dram_set_word(0, 0x11111111);
	sim_dram_set_word(0x40, 0x22222222);
	mar_i = 0;
	read_memory_i(0);
	read_memory_i(0);
	CHECK_EQ(mfc_i, NOT_COMPLETED);
	read_memory_i(1);			// flush
	CHECK_EQ(mfc_i, COMPLETED);
	// a new fetch starts from scratch and returns the new word
	CHECK_EQ(calls_until_read_i_completes(0x40), 5);
	CHECK_EQ(mbr_i, 0x22222222);
	CHECK_EQ(n_instruction_reads, 1);
}

TEST(consecutive_accesses_restart_the_counter){
	sim_setup(0);
	DRAM_READ_LATENCY = 3;
	for(int i = 0; i < 4; i++){
		CHECK_EQ(calls_until_read_completes(0x40000 + 4 * i, 0b010), 3);
	}
	CHECK_EQ(n_reads, 4);
}

TEST(trace_files_record_accesses){
	sim_setup(-1);
	trace = 1;
	open_trace_files("itrace.txt", "dtrace.txt");
	calls_until_read_i_completes(0x8);
	calls_until_read_completes(0x40010, 0b010);
	calls_until_write_completes(0x40020, 0b000, 1);
	fclose(memory_access_history_file_i);
	fclose(memory_access_history_file_d);
	CHECK_STR_EQ(test_read_file("itrace.txt"), "i 0x000008\n");
	CHECK_STR_EQ(test_read_file("dtrace.txt"), "r 0x040010\nw 0x040020\n");
}

TEST(trace_files_not_created_without_trace_flag){
	sim_setup(-1);
	trace = 0;
	open_trace_files("itrace.txt", "dtrace.txt");
	CHECK(access("itrace.txt", F_OK) != 0);
	CHECK(access("dtrace.txt", F_OK) != 0);
}

TEST(memory_dump_writes_whole_dram){
	sim_setup(-1);
	DRAM_SIZE = 4096;		// keep the dump small
	dram[0] = 0xAB;
	dram[4095] = 0xCD;
	display_memory("dump.bin");
	FILE * f = fopen("dump.bin", "rb");
	CHECK(f != NULL);
	if(!f) return;
	fseek(f, 0, SEEK_END);
	CHECK_EQ(ftell(f), 4096);
	unsigned char b[4096];
	rewind(f);
	CHECK_EQ(fread(b, 1, 4096, f), 4096);
	fclose(f);
	CHECK_EQ(b[0], 0xAB);
	CHECK_EQ(b[4095], 0xCD);
}

TEST(memory_dump_to_stdout_format){
	sim_setup(-1);
	DRAM_SIZE = 64;
	dram[0] = 0x01;
	dram[33] = 0xFF;
	fflush(stdout);
	int saved = dup(1);
	int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	dup2(fd, 1);
	display_memory(NULL);
	fflush(stdout);
	dup2(saved, 1);
	close(fd);
	CHECK_STR_EQ(test_read_file("out.txt"),
		"[00000000] : 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00\n"
		"[00000020] : 00 ff 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00\n");
}

TEST(free_instructions_releases_strings){
	sim_setup(-1);
	uint32_t w[] = {1, 2, 3};
	sim_load_words(0, w, 3);
	free_instructions();
	CHECK(instructions[0] == NULL);
	CHECK(instructions[2] == NULL);
}

TEST_SUITE("memory",
	T(ideal_memory_completes_in_one_call),
	T(dram_latency_is_the_number_of_calls),
	T(write_is_only_visible_after_completion),
	T(cancel_resets_instruction_read),
	T(consecutive_accesses_restart_the_counter),
	T(trace_files_record_accesses),
	T(trace_files_not_created_without_trace_flag),
	T(memory_dump_writes_whole_dram),
	T(memory_dump_to_stdout_format),
	T(free_instructions_releases_strings),
)
