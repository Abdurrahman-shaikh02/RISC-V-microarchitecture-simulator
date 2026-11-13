// src/cache/*: lookup, setters, eviction (FIFO/LRU, write back), the read and
// write paths through the hierarchy, and the cache statistics.
//
// Default machine (configure(NULL)), latencies in cycles:
//   L1-I/L1-D 2KB, 16B blocks, 2 way, 1/1,  write through, write allocate, FIFO
//   L2        8KB, 32B blocks, 4 way, 10/10, write back, no write allocate, FIFO
//   L3       16KB, 64B blocks, 8 way, 30/30, write back, no write allocate, FIFO
//   DRAM     200/200
// L1 sets are 1KB apart, L2 and L3 sets 2KB apart.
#include "sim_support.h"

#define A 0x40000u

// one full access as the memory ports do it: start (returns total cycles) + complete
static int rd(uint32_t addr){
	cycle++;
	int c = set_read_counter(addr);
	complete_read(addr);
	return c;
}

static int rd_i(uint32_t addr){
	cycle++;
	int c = set_read_i_counter(addr);
	complete_read_i(addr);
	return c;
}

static int wr(uint32_t addr){
	cycle++;
	int c = set_write_counter(addr);
	complete_write(addr);
	return c;
}

static void reshape(cache_level * c, int size, int block, int assoc){
	free(c->cache);
	c->cache_size = size;
	c->block_size = block;
	c->associativity = assoc;
	c->cache = calloc(size / 4, sizeof(cache_line));
}

//------------------------------------------------------------------------------
// geometry, lookup and setters
//------------------------------------------------------------------------------

TEST(tag_computation){
	// L1: 4 offset bits, 64 sets -> 6 index bits
	CHECK_EQ(get_tag(0x40000, 2048, 16, 2), 0x40000 >> 10);
	CHECK_EQ(get_tag(0x403FF, 2048, 16, 2), 0x40000 >> 10);
	CHECK_EQ(get_tag(0x40400, 2048, 16, 2), (0x40000 >> 10) + 1);
	// L3: 6 offset bits, 32 sets -> 5 index bits
	CHECK_EQ(get_tag(0x12345678, 16384, 64, 8), 0x12345678 >> 11);
	// direct mapped 64B: 4 sets
	CHECK_EQ(get_tag(0x40030, 64, 16, 1), 0x40030 >> 6);
}

TEST(find_block_states){
	sim_setup(1);
	CHECK_EQ(find_block(A, l1_d), 0);		// empty: can be placed
	CHECK_EQ(evict(A, l1_d), 0);			// allocate (not done yet)
	set_done(A, 1, l1_d);
	CHECK_EQ(find_block(A, l1_d), 1);
	CHECK_EQ(find_block(A + 12, l1_d), 1);		// same 16B block
	CHECK_EQ(find_block(A + 16, l1_d), 0);		// next block, a way is still free
}

TEST(find_block_reports_full_set_of_pending_blocks){
	sim_setup(1);
	evict(A, l1_d);				// both ways allocated and still pending
	evict(A + 0x400, l1_d);
	CHECK_EQ(find_block(A + 0x800, l1_d), -1);
	set_done(A, 1, l1_d);
	CHECK_EQ(find_block(A + 0x800, l1_d), 0);
}

TEST(find_block_on_pending_same_block_is_fatal){
	sim_setup(1);
	evict(A, l1_d);
	CHECK_FATAL(find_block(A, l1_d), "structural hazard waow");
}

TEST(setters_update_every_word_of_the_block){
	sim_setup(1);
	evict(A, l1_d);
	CHECK_EQ(set_dirty(A + 8, 1, l1_d), 0);
	CHECK_EQ(set_reference(A, 1234, l1_d), 0);
	cache_line * set0 = l1_d.cache;	// A maps to set 0, way 0
	for(int w = 0; w < 4; w++){
		CHECK_EQ(set0[w].valid, 1);
		CHECK_EQ(set0[w].dirty, 1);
		CHECK_EQ(set0[w].reference, 1234);
		CHECK_EQ(set0[w].addr, A + 4 * w);
	}
	CHECK_EQ(set0[4].valid, 0);		// way 1 untouched
}

TEST(setters_fail_for_missing_block){
	sim_setup(1);
	evict(A, l1_d);
	CHECK_EQ(set_valid(A + 0x400, 1, l1_d), -1);
	CHECK_EQ(set_dirty(A + 0x400, 1, l1_d), -1);
	CHECK_EQ(set_done(A + 0x400, 1, l1_d), -1);
	CHECK_EQ(set_reference(A + 0x400, 1, l1_d), -1);
}

TEST(copy_from_mem_sets_word_addresses){
	cache_line lines[4] = {0};
	copy_from_mem(0x1000, lines, 4);
	CHECK_EQ(lines[0].addr, 0x1000);
	CHECK_EQ(lines[3].addr, 0x100C);
}

TEST(touch_updates_reference_only_for_lru){
	sim_setup(1);
	evict(A, l1_d);
	set_reference(A, 5, l1_d);
	cycle = 99;
	l1_d.replacement_policy = 0;
	touch(A, l1_d);
	CHECK_EQ(l1_d.cache[0].reference, 5);
	l1_d.replacement_policy = 1;
	touch(A, l1_d);
	CHECK_EQ(l1_d.cache[0].reference, 99);
}

TEST(complete_below_marks_lower_levels_done){
	sim_setup(3);
	evict(A, l1_d);
	evict(A, l2);
	evict(A, l3);
	complete_below(A, l2);
	CHECK_EQ(find_block(A, l2), 1);
	CHECK_EQ(find_block(A, l3), 1);
	CHECK_FATAL(find_block(A, l1_d), "structural hazard");	// l1 untouched
	complete_below(A, l1_d);
	CHECK_EQ(find_block(A, l1_d), 1);
}

TEST(invalid_replacement_policy_is_fatal){
	sim_setup(1);
	l1_d.replacement_policy = 5;
	CHECK_FATAL(evict(A, l1_d), "Invalid replacement policy");
}

//------------------------------------------------------------------------------
// latency of the read path
//------------------------------------------------------------------------------

TEST(ideal_memory_costs_one_cycle){
	sim_setup(-1);
	CHECK_EQ(rd(A), 1);
	CHECK_EQ(wr(A), 1);
	CHECK_EQ(rd_i(0), 1);
	CHECK_EQ(l1_d_miss_r + l1_d_hit_r + l1_i_miss_r, 0);
}

TEST(no_cache_costs_dram_latency){
	sim_setup(0);
	DRAM_READ_LATENCY = 123;
	DRAM_WRITE_LATENCY = 45;
	CHECK_EQ(rd(A), 123);
	CHECK_EQ(rd(A), 123);
	CHECK_EQ(wr(A), 45);
	CHECK_EQ(rd_i(0), 123);
}

TEST(l1_only_read_miss_then_hit){
	sim_setup(1);
	CHECK_EQ(rd(A), 1 + 200);
	CHECK_EQ(rd(A), 1);
	CHECK_EQ(rd(A + 15), 1);
	CHECK_EQ(rd(A + 16), 201);
	CHECK_EQ(l1_d_miss_r, 2);
	CHECK_EQ(l1_d_hit_r, 2);
}

TEST(three_level_read_latencies){
	sim_setup(3);
	CHECK_EQ(rd(A), 1 + 10 + 30 + 200);	// misses everywhere
	CHECK_EQ(rd(A + 4), 1);			// L1 hit
	CHECK_EQ(rd(A + 16), 1 + 10);		// L1 miss, L2 hit (32B block)
	CHECK_EQ(rd(A + 32), 1 + 10 + 30);	// L2 miss, L3 hit (64B block)
	CHECK_EQ(l1_d_miss_r, 3);
	CHECK_EQ(l1_d_hit_r, 1);
	CHECK_EQ(l2_miss_r, 2);
	CHECK_EQ(l2_hit_r, 1);
	CHECK_EQ(l3_miss_r, 1);
	CHECK_EQ(l3_hit_r, 1);
}

TEST(two_level_read_latencies){
	sim_setup(2);
	CHECK_EQ(rd(A), 1 + 10 + 200);
	CHECK_EQ(rd(A + 16), 1 + 10);
	CHECK_EQ(rd(A + 32), 1 + 10 + 200);
	CHECK_EQ(l3_miss_r + l3_hit_r, 0);
}

TEST(instruction_reads_use_l1_i_and_share_l2){
	sim_setup(3);
	CHECK_EQ(rd_i(0), 241);
	CHECK_EQ(rd_i(4), 1);
	CHECK_EQ(l1_i_miss_r, 1);
	CHECK_EQ(l1_i_hit_r, 1);
	CHECK_EQ(rd(0), 1 + 10);		// L1-D misses, but the block is in the shared L2
	CHECK_EQ(l1_d_miss_r, 1);
}

//------------------------------------------------------------------------------
// write path
//------------------------------------------------------------------------------

TEST(write_through_write_allocate_default){
	sim_setup(3);
	// miss: allocate by reading (241) then write hit through to L2 (1 + 10)
	CHECK_EQ(wr(A), 241 + 1 + 10);
	CHECK_EQ(l1_d_miss_w, 1);
	CHECK_EQ(l1_d_hit_w, 0);
	CHECK_EQ(l1_d_miss_r, 0);		// the allocating read is not counted as a read miss
	CHECK_EQ(l2_hit_w, 1);
	// hit
	CHECK_EQ(wr(A), 1 + 10);
	CHECK_EQ(l1_d_hit_w, 1);
	// the block is now in L1
	CHECK_EQ(rd(A + 4), 1);
}

TEST(write_through_l1_only_goes_to_dram){
	sim_setup(1);
	CHECK_EQ(wr(A), 201 + 1 + 200);
	CHECK_EQ(wr(A), 1 + 200);
}

TEST(no_write_allocate_bypasses_l1){
	sim_setup(1);
	l1_d.write_miss_policy = 0;
	CHECK_EQ(wr(A), 200);
	CHECK_EQ(l1_d_miss_w, 1);
	CHECK_EQ(rd(A), 201);			// still not cached
}

TEST(no_write_allocate_l1_writes_to_l2){
	sim_setup(2);
	l1_d.write_miss_policy = 0;
	// L2 is also no write allocate: straight to dram
	CHECK_EQ(wr(A), 200);
	CHECK_EQ(l2_miss_w, 1);
	CHECK_EQ(rd(A), 211);
	l1_d.write_miss_policy = 1;
	CHECK_EQ(wr(A), 1 + 10);		// L1 hit, L2 hit (write back)
}

TEST(write_back_marks_dirty_and_writes_back_on_eviction){
	sim_setup(1);
	l1_d.write_policy = 1;
	CHECK_EQ(wr(A), 201 + 1);		// allocate, write hit stays in L1
	CHECK_EQ(wr(A + 0x400), 202);		// fills the other way of the set
	CHECK_EQ(wr(A), 1);
	// third block in the set: evicts A (FIFO, dirty) -> extra dram write
	CHECK_EQ(rd(A + 0x800), 1 + 200 + 200);
	CHECK_EQ(l1_d_writebacks, 1);
}

TEST(write_through_eviction_has_no_writeback){
	sim_setup(1);
	wr(A);
	wr(A + 0x400);
	CHECK_EQ(rd(A + 0x800), 201);
	CHECK_EQ(l1_d_writebacks, 0);
}

TEST(clean_block_eviction_in_write_back_cache_is_free){
	sim_setup(1);
	l1_d.write_policy = 1;
	rd(A);
	rd(A + 0x400);
	CHECK_EQ(rd(A + 0x800), 201);
	CHECK_EQ(l1_d_writebacks, 0);
}

TEST(l2_dirty_eviction_writes_back_to_l3){
	sim_setup(3);
	// A + k*2KB all map to L2 set 0 (4 ways); the L1 write through makes them dirty in L2
	for(int k = 0; k < 4; k++){
		wr(A + k * 0x800);
	}
	CHECK_EQ(l2_writebacks, 0);
	// 5th block: L3 miss (230), L2 evicts dirty A -> written back to L3 (hit, 30), +10, +1, then write hit 11
	CHECK_EQ(wr(A + 4 * 0x800), 230 + 30 + 10 + 1 + 1 + 10);
	CHECK_EQ(l2_writebacks, 1);
	CHECK_EQ(l3_hit_w, 1);
}

//------------------------------------------------------------------------------
// replacement policies
//------------------------------------------------------------------------------

TEST(fifo_evicts_oldest_insertion){
	sim_setup(1);
	l1_d.replacement_policy = 0;
	rd(A);
	rd(A + 0x400);
	CHECK_EQ(rd(A), 1);			// hit does not change FIFO order
	rd(A + 0x800);				// evicts A
	CHECK_EQ(rd(A + 0x400), 1);
	CHECK_EQ(rd(A), 201);
}

TEST(lru_evicts_least_recently_used){
	sim_setup(1);
	l1_d.replacement_policy = 1;
	rd(A);
	rd(A + 0x400);
	CHECK_EQ(rd(A), 1);			// A is now the most recent
	rd(A + 0x800);				// evicts A + 0x400
	CHECK_EQ(rd(A), 1);
	CHECK_EQ(rd(A + 0x400), 201);
}

TEST(direct_mapped_conflicts){
	sim_setup(1);
	reshape(&l1_d, 64, 16, 1);		// 4 sets, 64B apart
	CHECK_EQ(rd(A), 201);
	CHECK_EQ(rd(A + 16), 201);		// different set
	CHECK_EQ(rd(A + 64), 201);		// same set as A: evicts it
	CHECK_EQ(rd(A), 201);
	CHECK_EQ(rd(A + 16), 1);
}

TEST(single_set_cache){
	// associativity == number of blocks: one set (fully associative)
	// NOTE: works by accident today. index_bits = 0 makes the index computation
	// shift a uint32_t by 32 (undefined behaviour); at -O0 the bogus index is
	// 0x40000000 and only wraps back to 0 in the 32 bit offset multiplication.
	sim_setup(1);
	reshape(&l1_d, 64, 16, 4);
	for(int i = 0; i < 4; i++) CHECK_EQ(rd(A + 0x1000 * i), 201);
	for(int i = 0; i < 4; i++) CHECK_EQ(rd(A + 0x1000 * i + 4), 1);	// any word of the block hits
}

TEST(associativity_zero_means_fully_associative){
	// cache.h documents "associativity: 0 is fully associative"
	sim_setup(1);
	reshape(&l1_d, 64, 16, 0);
	CHECK_EQ(rd(A), 201);
	CHECK_EQ(rd(A), 1);
}

TEST(many_blocks_stress_hit_counts){
	// 64 words written then read twice (like the demo array program):
	// 2KB L1-D holds everything, so all reads hit
	sim_setup(3);
	for(int i = 0; i < 64; i++) wr(A + 4 * i);
	for(int p = 0; p < 2; p++)
		for(int i = 0; i < 64; i++) rd(A + 4 * i);
	CHECK_EQ(l1_d_miss_w, 16);
	CHECK_EQ(l1_d_hit_w, 48);
	CHECK_EQ(l1_d_hit_r, 128);
	CHECK_EQ(l1_d_miss_r, 0);
}

//------------------------------------------------------------------------------
// structural hazards
//------------------------------------------------------------------------------

TEST(set_read_counter_reports_full_pending_set){
	sim_setup(1);
	evict(A + 0x400, l1_d);
	evict(A + 0x800, l1_d);
	CHECK_EQ(set_read_counter(A), -1);
	CHECK_EQ(set_write_counter(A), -1);
}

TEST(memory_port_raises_structural_hazard_stall){
	// set_read_counter() returns -1, read_memory() computes counter = -1 - 1 = -2
	// and then tests counter == -1, so the hazard is never raised
	sim_setup(1);
	evict(A + 0x400, l1_d);
	evict(A + 0x800, l1_d);
	mar = A;
	read_memory(0b010);
	CHECK_EQ(STRUCTURAL_HAZARD_STALL, 1);
	CHECK_EQ(n_structural_hazards, 1);
	CHECK_EQ(n_read_d_stalls, 0);
}

TEST_SUITE("cache",
	T(tag_computation),
	T(find_block_states),
	T(find_block_reports_full_set_of_pending_blocks),
	T(find_block_on_pending_same_block_is_fatal),
	T(setters_update_every_word_of_the_block),
	T(setters_fail_for_missing_block),
	T(copy_from_mem_sets_word_addresses),
	T(touch_updates_reference_only_for_lru),
	T(complete_below_marks_lower_levels_done),
	T(invalid_replacement_policy_is_fatal),
	T(ideal_memory_costs_one_cycle),
	T(no_cache_costs_dram_latency),
	T(l1_only_read_miss_then_hit),
	T(three_level_read_latencies),
	T(two_level_read_latencies),
	T(instruction_reads_use_l1_i_and_share_l2),
	T(write_through_write_allocate_default),
	T(write_through_l1_only_goes_to_dram),
	T(no_write_allocate_bypasses_l1),
	T(no_write_allocate_l1_writes_to_l2),
	T(write_back_marks_dirty_and_writes_back_on_eviction),
	T(write_through_eviction_has_no_writeback),
	T(clean_block_eviction_in_write_back_cache_is_free),
	T(l2_dirty_eviction_writes_back_to_l3),
	T(fifo_evicts_oldest_insertion),
	T(lru_evicts_least_recently_used),
	T(direct_mapped_conflicts),
	T(single_set_cache),
	XFAIL(associativity_zero_means_fully_associative, "documented in cache.h, but n_sets = size / (block * 0) divides by zero"),
	T(many_blocks_stress_hit_counts),
	T(set_read_counter_reports_full_pending_set),
	XFAIL(memory_port_raises_structural_hazard_stall, "memory.c checks counter == -1 after counter = set_*_counter() - 1, the -1 hazard code becomes -2: hazard never flagged, stall stats decremented by 2"),
)
