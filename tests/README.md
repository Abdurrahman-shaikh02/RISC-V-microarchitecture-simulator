# Test suite

A safety net for refactoring the simulator: it pins down what the project does
**today** (results, timing and the user interface) so that any change in behaviour
during a refactor shows up as a failing test that says exactly what changed and where.

```
make test               # everything except the long C workloads (~7 s)
make test-unit          # white box tests of every module
make test-integration   # black box tests of ./assemble ./simulate ./cachesim
make test-slow          # everything + programs/c quick_sort / radix_sort
make test-golden        # regenerate golden files (only after an INTENTIONAL change)
```

`make test` ends with a summary table and exits non zero if any test failed.

## Two tiers

| tier | where | tests | tied to |
|---|---|---|---|
| unit | `tests/unit/sim`, `tests/unit/asm` | every module, function by function | the internal C API |
| integration | `tests/integration` | the three programs, through their command line and output files | the user interface only |

**During a refactor, the integration tier is the contract**: it must stay green
without being edited. The unit tier calls internal functions (`ex_stage()`,
`find_block()`, `get_tokens()`, ...). When a refactor renames or reshapes those,
update the unit tests together with the code, keeping what they assert.

## Reading the output

```
  PASS  test_name
  FAIL  test_name
      tests/unit/sim/test_cache.c:212: CHECK_EQ(rd(A + 16), 1 + 10) failed
        expected: 0x0000000b (11)
        actual:   0x0000000c (12)
  XFAIL test_name  (known bug: ...)       known bug, does not fail the run
  XPASS test_name  (known bug seems FIXED) the bug got fixed: remove the XFAIL marker
```

Each unit test runs in its own forked process and temporary directory, so a
crash or a `log_fatal()` → `exit(1)` inside the simulator is reported as a failure
of that one test (with the last lines the simulator printed) instead of killing
the run. Integration failures show the configuration, the command and a diff.

Running one suite or one test:

```
build/tests/sim/test_cache                 # one unit suite
build/tests/sim/test_cache lru fifo -v     # tests matching "lru" or "fifo", show simulator output and xfail details
python3 tests/integration/run.py programs:: timing::isa_hazards -v --keep
```

## Unit tests (`tests/unit`)

| suite | covers |
|---|---|
| `test_alu` | arithmetic/logic units, shifts (5 bit masking), comparators, immediate generation of every format |
| `test_control` | decoder: format, write back, memory access codes, branch/jump marking, system instructions, illegal encodings |
| `test_branch` | static predictors (not taken, taken, BTFN), `predict()`/`store()` dispatch, dynamic predictor stubs |
| `test_forwarding` | `forward_ex`/`forward_ma` priorities and forced stalls, x0, the no-forwarding hazard detector |
| `test_ex` | every RV32I instruction decoded by the real ID stage and executed by the real EX stage |
| `test_stages` | IF (fetch, bubbles while memory is busy), ID (branch redirect, flags reset, hazard stall), MA (branch resolution, loads/stores of every width, illegal accesses, latency), WB |
| `test_memory` | memory ports: latency counters, cancel on flush, traces, memory dump |
| `test_cache` | tags, lookup, setters, FIFO/LRU eviction, write back/through, (no) write allocate, latencies through 1, 2 and 3 levels, statistics |
| `test_config` | default machine, JSON overrides (full, partial, invalid, missing file) |
| `test_loaders` | the object file loader and the ELF loader (hand made ELF images, every rejection path) |
| `test_output` | exact text of the statistics report and register dumps, logger levels |
| `asm/test_parser` | tokenizer, comments, labels, syntax errors |
| `asm/test_semantics` | instruction table, operand counts, register/immediate/label/memory operand ranges, symbol table |
| `asm/test_encoding` | machine code of every instruction against encoders written from the ISA spec (`tests/framework/rv32.h`) |

## Integration tests (`tests/integration/run.py`)

| group | what it checks |
|---|---|
| `assembler_cli` | argument handling, error messages and line numbers, plain/verbose output format, golden object files |
| `assembler_vs_gnu_as` | every test program assembled by GNU `as`: identical machine code (needs riscv64-unknown-elf binutils) |
| `simulator_cli` | every argument error, which output files each flag creates, stdout layout, missing/invalid inputs |
| `programs` | every program in `integration/asm` under **13 machine configurations** (ideal memory, no cache, 1/2/3 levels, write back, LRU, no write allocate, tiny caches, forwarding off, every predictor); registers and memory must be the expected ones under all of them |
| `timing` | all 36 statistics of every program × configuration run, against `golden/timing.json` |
| `documented` | the numbers published in `collegereport/demo/README.md` (cycles, misses, hits) |
| `golden_outputs` | `pipevis.txt`, register dumps, memory dumps, instruction/data traces against `golden/` |
| `cachesim` | trace replay output, built in latencies, cross check with the simulator's L1-D counters |
| `c_programs` | C programs compiled with `essentials/` (crt0, linker script, soft mul/div) at -O0 and -O2, run under 3 configurations, result compared with the same program compiled for the host (needs riscv64-unknown-elf-gcc) |

### Writing a test program

Add a `.s` file (or a hand written verbose `.obj` file) to `tests/integration/asm`
and state the expected results in comments:

```
// EXPECT r5 = 0x78
// EXPECT mem32[0x40000] = 55        (mem8[...] for one byte; DRAM is 1MB in these configs)
// EXPECT exit = 1                    (default 0)
// EXPECT output: Unaligned word access
// CONFIGS: ideal full                (default: all configurations)
// XFAIL: why this is a known bug
```

Then run `make test-golden` to record its timing/output, and check `git diff tests/integration/golden`.
Do not put `]` in comments of `.obj` files (see the custom loader note below).

### Golden files

`tests/integration/golden` holds the output of the current build: timing
statistics, pipeline diagrams, dumps, traces, assembler and cachesim output. They
were generated from commit `7bab86b` (branch `refactor`). A pure refactor must not change any of
them. If a change of timing or format is intended, run `make test-golden` and
review the diff before committing it. C program timing depends on the compiler
version; it is only compared when the installed riscv64-unknown-elf-gcc matches
the one recorded in `golden/timing_c.json`.

## Known bugs found while writing the tests

Tracked as `XFAIL` (the test asserts the correct behaviour):

| test | bug |
|---|---|
| `control: standard_ebreak_encoding_is_a_breakpoint`, `programs: std_ebreak` | the decoder separates ecall/ebreak by funct7 instead of imm: the spec `ebreak` (0x00100073, what gcc emits) runs as an `ecall` |
| `asm_encoding: ebreak_uses_standard_encoding` | the assembler emits ebreak as 0x02000073 (the simulator expects exactly this, so fix both together) |
| `execute: jalr_clears_the_lowest_target_bit`, `programs: isa_jalr_odd` | `jalr` does not clear bit 0 of the target: odd targets crash with an unaligned fetch |
| `cache: memory_port_raises_structural_hazard_stall` | `memory.c` does `counter = set_*_counter() - 1` and then tests `counter == -1`, but the hazard code -1 becomes -2: structural hazards are never flagged and the stall statistics are decremented by 2 (wrap around) |
| `cache: associativity_zero_means_fully_associative` | `cache.h` documents associativity 0 as fully associative, it divides by zero (SIGFPE) |
| `assembler_cli: plain_with_output_file` | `assemble <in> <out>` returns -1 without writing anything (stray `return -1` in the `argc == 3` branch of `main.c`) |

Pinned as current behaviour, worth fixing (deliberately, together with their tests):

- **Single set caches work by accident.** With one set, `index_bits` is 0 and the index is computed with a 32 bit shift (undefined behaviour). At `-O0` the index is 0x40000000 and only wraps to 0 in the 32 bit offset multiplication (`test_cache: single_set_cache`).
- **The object file loader takes any line containing `]` for an instruction** and `sscanf("%x")` accepts a leading word like `error` as the hex number `0xe`.
- **`cachesim` always exits with status 1**, even on success (`cachesim` group).
- **`store()` in `branch_prediction.c` has no `break`s**, so `TWO_BIT_SWITCH` falls through into `HISTORY_TABLE`. Harmless while the dynamic predictors are empty stubs, a bug once they are implemented.
- **`log_fatal()` expands to two statements**, so `if(x) log_fatal("...");` without braces would always print.
- **Byte/halfword reads near the end of DRAM** read `dram[mar + 1..3]` before checking bounds. `mar < 0` checks on unsigned values are dead code.
- **`programs/assembly/*.s` are stale**: they store to 0x900, which is inside the text segment in the current memory map, so they all stop with "attempt to write to text segment" (they are not used by the tests).
- **Makefile**: `simulate: simulate` is a circular phony rule (make drops it). The test targets also turned on header dependency tracking (`-MMD -MP`) so that a refactor that only touches a header rebuilds everything that includes it.
