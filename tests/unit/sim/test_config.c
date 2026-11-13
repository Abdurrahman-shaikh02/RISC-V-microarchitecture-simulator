// src/utils/configure.c + src/utils/json_loader.c: default machine and JSON overrides
#include "sim_support.h"

static void check_cache(cache_level c, int level, int size, int block, int assoc, int rl, int wl, int wp, int wmp, int rp){
	CHECK_EQ(c.level, level);
	CHECK_EQ(c.cache_size, size);
	CHECK_EQ(c.block_size, block);
	CHECK_EQ(c.associativity, assoc);
	CHECK_EQ(c.read_latency, rl);
	CHECK_EQ(c.write_latency, wl);
	CHECK_EQ(c.write_policy, wp);
	CHECK_EQ(c.write_miss_policy, wmp);
	CHECK_EQ(c.replacement_policy, rp);
	CHECK(c.cache != NULL);
}

static void check_defaults(void){
	CHECK_EQ(CACHE_LEVELS, 3);
	CHECK_EQ(L1_SPLIT, 1);
	check_cache(l1_d, 1, 2048, 16, 2, 1, 1, 0, 1, 0);
	check_cache(l1_i, 1, 2048, 16, 2, 1, 1, 0, 1, 0);
	check_cache(l2, 2, 8192, 32, 4, 10, 10, 1, 0, 0);
	check_cache(l3, 3, 16384, 64, 8, 30, 30, 1, 0, 0);
	CHECK_EQ(DRAM_READ_LATENCY, 200);
	CHECK_EQ(DRAM_WRITE_LATENCY, 200);
	CHECK_EQ(DRAM_SIZE, 1024 * 1024 * 1024);
	CHECK_EQ(FORWARDING_SWITCH, 1);
	CHECK_EQ(BRANCH_PREDICTION_STRATEGY, NOT_TAKEN);
	CHECK_EQ(text_segment_limit, 0x0001FFFF);
	CHECK_EQ(rodata_segment_limit, 0x0003FFFF);
	CHECK_EQ(total_number_of_instructions, 0x20000 / 4);
	CHECK(dram != NULL);
	CHECK(instructions != NULL);
}

TEST(defaults_without_config_file){
	sim_quiet();
	configure(NULL);
	check_defaults();
}

TEST(dram_and_caches_start_zeroed){
	sim_quiet();
	configure(NULL);
	CHECK_EQ(dram[0] | dram[0x40000] | dram[DRAM_SIZE - 1], 0);
	CHECK_EQ(l1_d.cache[0].valid | l2.cache[10].valid | l3.cache[100].valid, 0);
	CHECK(instructions[0] == NULL);
}

TEST(full_json_overrides_everything){
	sim_quiet();
	test_write_file("cfg.json",
		"{\"CACHE_LEVELS\": 2, \"L1_SPLIT\": 0,"
		" \"l1_d\": {\"cache_size\": 64, \"block_size\": 16, \"associativity\": 1, \"read_latency\": 2, \"write_latency\": 3,"
		"   \"write_miss_policy\": 0, \"replacement_policy\": 1, \"write_policy\": 1},"
		" \"l1_i\": {\"cache_size\": 128, \"block_size\": 32, \"associativity\": 4, \"read_latency\": 4, \"write_latency\": 5,"
		"   \"write_miss_policy\": 0, \"replacement_policy\": 1, \"write_policy\": 1},"
		" \"l2\": {\"cache_size\": 1024, \"block_size\": 64, \"associativity\": 2, \"read_latency\": 6, \"write_latency\": 7,"
		"   \"write_miss_policy\": 1, \"replacement_policy\": 1, \"write_policy\": 0},"
		" \"l3\": {\"cache_size\": 4096, \"block_size\": 128, \"associativity\": 16, \"read_latency\": 8, \"write_latency\": 9,"
		"   \"write_miss_policy\": 1, \"replacement_policy\": 1, \"write_policy\": 0},"
		" \"DRAM_READ_LATENCY\": 11, \"DRAM_WRITE_LATENCY\": 12, \"DRAM_SIZE\": 1048576,"
		" \"FORWARDING_SWITCH\": 0, \"BRANCH_PREDICTION_STRATEGY\": 2,"
		" \"text_segment_limit\": 65535, \"rodata_segment_limit\": 131071}");
	configure("cfg.json");
	CHECK_EQ(CACHE_LEVELS, 2);
	CHECK_EQ(L1_SPLIT, 0);
	check_cache(l1_d, 1, 64, 16, 1, 2, 3, 1, 0, 1);
	check_cache(l1_i, 1, 128, 32, 4, 4, 5, 1, 0, 1);
	check_cache(l2, 2, 1024, 64, 2, 6, 7, 0, 1, 1);
	check_cache(l3, 3, 4096, 128, 16, 8, 9, 0, 1, 1);
	CHECK_EQ(DRAM_READ_LATENCY, 11);
	CHECK_EQ(DRAM_WRITE_LATENCY, 12);
	CHECK_EQ(DRAM_SIZE, 1048576);
	CHECK_EQ(FORWARDING_SWITCH, 0);
	CHECK_EQ(BRANCH_PREDICTION_STRATEGY, OFFSET);
	CHECK_EQ(text_segment_limit, 65535);
	CHECK_EQ(rodata_segment_limit, 131071);
	CHECK_EQ(total_number_of_instructions, 65536 / 4);
}

TEST(partial_json_only_overrides_given_keys){
	sim_quiet();
	test_write_file("cfg.json", "{\"CACHE_LEVELS\": -1, \"l2\": {\"read_latency\": 99}}");
	configure("cfg.json");
	CHECK_EQ(CACHE_LEVELS, -1);
	CHECK_EQ(l2.read_latency, 99);
	CHECK_EQ(l2.write_latency, 10);
	CHECK_EQ(l2.cache_size, 8192);
	CHECK_EQ(l1_d.cache_size, 2048);
	CHECK_EQ(DRAM_READ_LATENCY, 200);
}

TEST(cache_levels_cannot_be_overridden){
	sim_quiet();
	test_write_file("cfg.json", "{\"l1_d\": {\"level\": 3}, \"l3\": {\"level\": 1}}");
	configure("cfg.json");
	CHECK_EQ(l1_d.level, 1);
	CHECK_EQ(l3.level, 3);
}

TEST(non_numeric_and_unknown_keys_are_ignored){
	sim_quiet();
	test_write_file("cfg.json", "{\"CACHE_LEVELS\": \"two\", \"bogus\": 5, \"l1_d\": {\"cache_size\": [1]}}");
	configure("cfg.json");
	check_defaults();
}

TEST(caches_are_allocated_with_configured_size){
	sim_quiet();
	test_write_file("cfg.json", "{\"l1_d\": {\"cache_size\": 64, \"block_size\": 16, \"associativity\": 1}}");
	configure("cfg.json");
	// 64 bytes = 16 words; touching the last word must be legal
	CHECK_SURVIVES(l1_d.cache[15].valid = 1);
}

TEST(missing_config_file_falls_back_to_defaults){
	char out[4096];
	int st;
	RUN_ISOLATED(out, st, { sim_quiet(); configure("does_not_exist.json"); });
	CHECK_EQ(st, 0);
	CHECK_CONTAINS(out, "Could not open config file does_not_exist.json");
	sim_quiet();
	configure("does_not_exist.json");
	check_defaults();
}

TEST(invalid_json_falls_back_to_defaults){
	test_write_file("cfg.json", "{\"CACHE_LEVELS\": 1,,,");
	char out[4096];
	int st;
	RUN_ISOLATED(out, st, { sim_quiet(); configure("cfg.json"); });
	CHECK_EQ(st, 0);
	CHECK_CONTAINS(out, "Invalid JSON configuration.");
	sim_quiet();
	configure("cfg.json");
	check_defaults();
}

TEST(display_config_lists_settings){
	sim_quiet();
	configure(NULL);
	char out[8192];
	int st;
	RUN_ISOLATED(out, st, display_config());
	CHECK_EQ(st, 0);
	CHECK_CONTAINS(out, "CACHE_LEVELS = 3");
	CHECK_CONTAINS(out, "DRAM_SIZE = 1073741824");
	CHECK_CONTAINS(out, "text_segment_limit = 0x0001FFFF");
	CHECK_CONTAINS(out, "total_number_of_instructions = 32768");
}

TEST_SUITE("config",
	T(defaults_without_config_file),
	T(dram_and_caches_start_zeroed),
	T(full_json_overrides_everything),
	T(partial_json_only_overrides_given_keys),
	T(cache_levels_cannot_be_overridden),
	T(non_numeric_and_unknown_keys_are_ignored),
	T(caches_are_allocated_with_configured_size),
	T(missing_config_file_falls_back_to_defaults),
	T(invalid_json_falls_back_to_defaults),
	T(display_config_lists_settings),
)
