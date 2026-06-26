###############################################################################
# Compiler
###############################################################################

CC := gcc

CFLAGS := -g -Iinclude	# Use -Wall and -Wextra and make the code bulletproof!!!
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
src/cpu/alu.c \
src/cpu/control.c \
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
src/stats/stats.c \
src/utils/logger.c \
src/utils/configure.c \
src/utils/utility.c

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
# Cleaning
###############################################################################

clean:
	rm -rf $(BUILD)
	rm -f assemble simulate cachesim
	rm -f log.log output.txt data_trace.txt instruction_trace.txt

rebuild: clean all
