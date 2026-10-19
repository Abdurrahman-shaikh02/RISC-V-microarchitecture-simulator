###############################################################################
# Compiler
###############################################################################

CC := gcc

CFLAGS := -g -Iinclude -MMD -MP	# Use -Wall and -Wextra and make the code bulletproof!!!
LDLIBS := -lm

BUILD := build

###############################################################################
# Assembler
###############################################################################

ASM_SRC := \
src/assembler/core/main.c \
src/assembler/core/opcode_generator.c \
src/assembler/core/parser.c \
src/assembler/core/semantics_analyzer.c \
src/assembler/core/symbol_table.c \
src/assembler/core/syntax_analyzer.c \
src/assembler/log/logger.c

ASM_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(ASM_SRC))

###############################################################################
# CPU Simulator
###############################################################################

SIM_SRC := \
src/main.c \
src/execution/alu.c \
src/execution/control.c \
src/execution/simulator.c \
src/pipeline/if.c \
src/pipeline/id.c \
src/pipeline/ex.c \
src/pipeline/ma.c \
src/pipeline/wb.c \
src/memory/memory.c \
src/cache/cache.c \
src/cache/read.c \
src/cache/write.c \
src/cache/setters.c \
src/cache/eviction.c \
src/branch/branch_prediction.c \
src/branch/static_prediction.c \
src/branch/dynamic_prediction.c \
src/forwarding/forwarding.c\
src/stats/stats.c \
src/utils/logger.c \
src/utils/configure.c \
src/utils/utility.c \
src/utils/custom_loader.c \
src/utils/elf_loader.c \
src/utils/cJSON.c \
src/utils/json_loader.c

SIM_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(SIM_SRC))

###############################################################################
# Cache Simulator
###############################################################################

CACHE_SRC := \
src/cache/cachesim.c \
src/cache/cache.c \
src/cache/read.c \
src/cache/write.c \
src/cache/setters.c \
src/cache/eviction.c \
src/stats/stats.c \
src/utils/logger.c

CACHE_OBJ := $(patsubst %.c,$(BUILD)/%.o,$(CACHE_SRC))

###############################################################################
# Phony Targets
###############################################################################

.PHONY: all assembler simulate cache clean rebuild

all: assemble simulate cachesim

assembler: assemble

simulate: simulate

cache: cachesim

###############################################################################
# Linking
###############################################################################

assemble: $(ASM_OBJ)
	$(CC) $(ASM_OBJ) -o $@

simulate: $(SIM_OBJ)
	$(CC) $(SIM_OBJ) -o $@ $(LDLIBS)

cachesim: $(CACHE_OBJ)
	$(CC) $(CACHE_OBJ) -o $@ $(LDLIBS)

###############################################################################
# Compilation
###############################################################################

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

###############################################################################
# Tests (see tests/README.md)
###############################################################################

TEST_BUILD := $(BUILD)/tests
TEST_CFLAGS := $(CFLAGS) -Itests/framework

# everything except the files that define main()
SIM_LIB_OBJ := $(filter-out $(BUILD)/src/main.o,$(SIM_OBJ))
ASM_LIB_OBJ := $(filter-out $(BUILD)/src/assembler/core/main.o,$(ASM_OBJ))

SIM_UNIT_SRC := $(wildcard tests/unit/sim/test_*.c)
ASM_UNIT_SRC := $(wildcard tests/unit/asm/test_*.c)
SIM_UNIT_BIN := $(patsubst tests/unit/sim/%.c,$(TEST_BUILD)/sim/%,$(SIM_UNIT_SRC))
ASM_UNIT_BIN := $(patsubst tests/unit/asm/%.c,$(TEST_BUILD)/asm/%,$(ASM_UNIT_SRC))
UNIT_BIN := $(SIM_UNIT_BIN) $(ASM_UNIT_BIN)

.PHONY: test test-unit test-integration test-slow test-golden

# all tests: unit + integration (the default, fast)
test: $(UNIT_BIN) assemble simulate cachesim
	@tests/run_tests.sh all $(UNIT_BIN)

# white box tests of every module (tied to the internal C API)
test-unit: $(UNIT_BIN)
	@tests/run_tests.sh unit $(UNIT_BIN)

# black box tests of ./assemble ./simulate ./cachesim (the user interface)
test-integration: assemble simulate cachesim
	@tests/run_tests.sh integration

# everything, including the long running C workloads
test-slow: $(UNIT_BIN) assemble simulate cachesim
	@tests/run_tests.sh slow $(UNIT_BIN)

# regenerate the golden files after an INTENTIONAL change of timing or output
test-golden: assemble simulate cachesim
	@python3 tests/integration/run.py --update-golden --slow

$(TEST_BUILD)/sim/%: tests/unit/sim/%.c $(TEST_BUILD)/sim/sim_support.o $(SIM_LIB_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -Itests/unit/sim $< $(TEST_BUILD)/sim/sim_support.o $(SIM_LIB_OBJ) -o $@ $(LDLIBS)

$(TEST_BUILD)/sim/sim_support.o: tests/unit/sim/sim_support.c
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -Itests/unit/sim -c $< -o $@

$(TEST_BUILD)/asm/%: tests/unit/asm/%.c $(ASM_LIB_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $< $(ASM_LIB_OBJ) -o $@ $(LDLIBS)

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)

###############################################################################
# Cleaning
###############################################################################

clean:
	rm -rf $(BUILD)
	rm -f assemble simulate cachesim
	rm -f log.log output.txt data_trace.txt instruction_trace.txt

rebuild: clean all
