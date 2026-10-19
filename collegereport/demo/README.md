# RISC-V Microarchitecture Simulator – live demo

A short, hands-on walkthrough: the assembler, the simulator, its output files,
and how changing the configuration changes the timing (never the result).
Every command is typed by hand; there are no wrapper scripts.

```
demo/
├── asm/        tiny assembly programs for the assembler
├── c/          small C programs (copied from programs/c)
├── configs/    ready-made simulator configurations (JSON)
└── run/        scratch directory you run everything from (create it)
```

The expected numbers below were measured on this build; if one differs slightly
after a code change, that is fine. The results (register values) must not change.

---

## 0. Build and set up

```bash
# from the repository root
make                                  # builds ./assemble, ./simulate, ./cachesim

cd collegereport/demo
mkdir -p run && cd run                # the simulator writes its output files into the current directory

# short names for the three tools (optional, saves typing)
A=../../../assemble
S=../../../simulate
CS=../../../cachesim
```

Files the simulator can produce in `run/`:

| Flag | Output file | What it is |
|---|---|---|
| `-r` | `register_dump`, `internal_register_dump` | the 32 registers, then pc/ir/mar/mbr and pipeline-register contents |
| `-p` | `pipevis.txt` | one line per cycle: the instruction in IF/ID, ID/EX, EX/MA, MA/WB and WB/IF |
| `-m` | `memory_dump.bin` | the whole simulated DRAM as raw bytes |
| `-t` | `instruction_trace.txt`, `data_trace.txt` | every memory address accessed, in order |
| `-l` | `log.log` | the simulator's internal log |
| always | statistics on the terminal | caches, stalls, branches, flushes, forwarding, CPI inputs |

---

## 1. The assembler

### 1.1 Assemble a program

```bash
$A -v ../asm/01_basics.s basics.obj
cat basics.obj
```

`-v` writes the *verbose* object file, which is the format the simulator loads.
Each line is the instruction index, its address, the 32-bit machine code and the
source line:

```
[0] 00000000   00c00093           addi    r1, r0, 12
[1] 0x000004   00500113           addi    r2, r0, 5
[2] 0x000008   002081b3           add     r3, r1, r2
...
[13] 0x000034   00000893           addi    r17, r0, 0
[14] 0x000038   00000073           ecall
```

Points to make:
- `00c00093` is `addi r1, r0, 12` in I-format: imm = 0x00c, rs1 = 0, funct3 = 0, rd = 1, opcode = 0010011.
- Programs end with `addi r17, r0, 0` + `ecall`: a7 (r17) = 0 is the simulator's *halt* system call.

### 1.2 Labels and branches (two-pass assembly)

```bash
$A -v ../asm/02_sum_loop.s sum.obj
cat sum.obj
```

The `bne r2, r3, loop` line refers to a label defined *earlier*; the first pass
builds the symbol table and the second pass turns `loop` into a PC-relative
offset (look for the negative offset in its encoding).

### 1.3 Error detection

```bash
$A -v ../asm/05_syntax_error.s err.obj
```

Expected:

```
Invalid operation name
Syntax error at line number : 10
[FATAL]: Invalid syntax encountered
```

(`mul` belongs to the M extension, which this RV32I core does not implement.)

Assemble the rest now:

```bash
$A -v ../asm/03_hazards.s hazards.obj
$A -v ../asm/04_array.s   array.obj
```

---

## 2. Running assembly on the simulator

### 2.1 Registers and the pipeline diagram

```bash
$S -a basics.obj -c ../configs/01_ideal.json -r -p
cat register_dump
head -20 pipevis.txt
```

`register_dump` should match the comment block at the top of `01_basics.s`
(r3 = 0x11, r4 = 7, r8 = 0x60, r11 = 1, r12 = 0, r13 = 0x12345000, …).
In `pipevis.txt`, each instruction moves one column to the right per cycle:
that is the five-stage pipeline. 14 instructions finish in 18 cycles
(4 cycles to fill the pipeline).

### 2.2 Memory dump

```bash
$S -a sum.obj -c ../configs/10_asm_memdump.json -r -m
grep r01 register_dump                 # r01 : 00000037   (55)
xxd -s 0x40000 -l 16 memory_dump.bin   # 00040000: 3700 0000 ...
```

The program stores the sum at 0x40000, the first address after the text (0–128 KB)
and read-only data (128–256 KB) segments. `10_asm_memdump.json` shrinks DRAM to
1 MB so the dump file is only 1 MB. (The other configs keep 1 GB because compiled
C programs put their stack at the top of 1 GB.)

### 2.3 Data hazards: forwarding on vs off

```bash
$S -a hazards.obj -c ../configs/01_ideal.json -r -p
cat pipevis.txt                        # 14 cycles

$S -a hazards.obj -c ../configs/02_ideal_no_forwarding.json -r -p
cat pipevis.txt                        # 28 cycles
```

| Config | Cycles | What to point at in `pipevis.txt` |
|---|---|---|
| forwarding ON | **14** | dependent `add`s go back-to-back; one bubble only for the load-use pair (`lw r5` → `add r6, r5, r5`) |
| forwarding OFF | **28** | every dependent instruction sits in IF/ID for 4 cycles while bubbles drain |

`register_dump` is identical in both runs (r6 = 0x78 = 120). Timing changes, results don't.
The statistics also show `Forwards to EX` / `Forwards to MA` being non-zero only when forwarding is on.

### 2.4 Branch prediction

The loop branch in `02_sum_loop.s` is taken 9 times and falls through once.

```bash
$S -a sum.obj -c ../configs/01_ideal.json            # always not-taken
$S -a sum.obj -c ../configs/03_ideal_always_taken.json
$S -a sum.obj -c ../configs/04_ideal_btfn.json       # backward taken, forward not taken
```

Look at `Branch Misses`, `Flushes from EX` and `Total Cycles`:

| Predictor | Branch misses | Cycles |
|---|---|---|
| not-taken | 9 | 58 |
| always taken | 1 | 51 |
| BTFN | 1 | 51 |

On the bigger array loop (`array.obj`) the difference is 189 vs 3 misses (1417 vs 1234 cycles).

### 2.5 Caches

`04_array.s` writes 64 words (256 bytes), then reads them twice.

```bash
$S -a array.obj -c ../configs/01_ideal.json          # ideal memory: every access 1 cycle
$S -a array.obj -c ../configs/05_no_cache.json       # no caches: 200-cycle DRAM for everything
$S -a array.obj -c ../configs/06_l1_only.json
$S -a array.obj -c ../configs/07_full_hierarchy.json # default: L1 + L2 + L3
$S -a array.obj -c ../configs/08_full_hierarchy_writeback.json
$S -a array.obj -c ../configs/09_tiny_l1d.json       # 64-byte direct-mapped L1-D
```

| Config | Cycles |
|---|---|
| ideal memory | 1,417 |
| no caches | 220,387 |
| L1 only | 18,231 |
| L1 + L2 + L3 (default) | 3,541 |
| default, L1-D write-back | 2,901 |
| default with 64 B L1-D | 3,861 |

And the L1-D counters:

| Config | Read hits | Read misses | Write misses |
|---|---|---|---|
| default 2 KB L1-D | 128 | **0** | 16 |
| 64 B L1-D | 96 | **32** | 16 |

With 2 KB, the 16 write misses of the fill loop bring the array in (write-allocate),
so both read passes hit. With 64 B the array does not fit, so each pass misses once
per 16-byte block (16 × 2 = 32).

### 2.6 Address traces and the stand-alone cache simulator

```bash
$S -a array.obj -c ../configs/07_full_hierarchy.json -t
head data_trace.txt                    # w 0x040000, w 0x040004, ...
$CS < data_trace.txt | head            # "<access no> : <cycles for that access>"
$CS < data_trace.txt | grep "L1 D"     # same L1-D hit/miss counts as the full simulator (128 / 0 / 48 / 16)
```

`cachesim` replays a trace through the same cache code (with the built-in default
geometry), which is handy for studying the memory system in isolation.

---

## 3. Running compiled C programs

The C programs are in `c/` (copies of the ones in `programs/c`):

| File | `main` returns |
|---|---|
| `factorial.c` | 5! = 120 |
| `nth_fibonacci.c` | fib(20) = 6765 |
| `test_matrix_multiplication.c` | 1 (the product is left in the global `C[][]`) |
| `quick_sort.c` | 0 (quicksorts the 16,384 numbers in `arr[]` in place) |
| `radix_sort.c` | 0 (sorts the same 16,384 numbers with a byte-wise radix sort) |

Compile with the project's start-up code, linker script and software multiply/divide helpers:

```bash
P=../c
E=../../../essentials
CC="riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 -nostdlib -ffreestanding -T $E/linker.ld $E/crt0.S"

$CC -O0 $P/factorial.c                  $E/stdlib/*.c -o factorial.elf
$CC -O2 $P/nth_fibonacci.c              $E/stdlib/*.c -o fib.elf
$CC -O2 $P/test_matrix_multiplication.c $E/stdlib/*.c -o matmul.elf
$CC -O2 $P/quick_sort.c                 $E/stdlib/*.c -o qsort.elf
$CC -O2 $P/radix_sort.c                 $E/stdlib/*.c -o radix.elf
```

(`factorial.c` is compiled at `-O0` on purpose: at `-O2` GCC computes 5! at
compile time and the program shrinks to six instructions.)

`main`'s return value ends up in a0 = **r10**:

```bash
$S -e factorial.elf -c ../configs/01_ideal.json -r -p
grep r10 register_dump                 # r10 : 00000078  (5! = 120)
less pipevis.txt                       # watch the recursive calls, jal/jalr flushes, __mulsi3

$S -e fib.elf -c ../configs/01_ideal.json -r
grep r10 register_dump                 # r10 : 00001a6d  (fib(20) = 6765)

$S -e matmul.elf -c ../configs/01_ideal.json -r
grep r10 register_dump                 # r10 : 00000001  (main returns 1; the product is in the global C[][])
```

### 3.1 Same program, different machines

```bash
$S -e fib.elf -c ../configs/01_ideal.json
$S -e fib.elf -c ../configs/02_ideal_no_forwarding.json
$S -e fib.elf -c ../configs/07_full_hierarchy.json
$S -e fib.elf -c ../configs/08_full_hierarchy_writeback.json
```

fib(20), 135,641 instructions every time:

| Config | Cycles | CPI |
|---|---|---|
| ideal, forwarding on | 164,385 | 1.21 |
| ideal, forwarding off | 214,929 | 1.58 |
| default hierarchy (write-through L1-D) | 488,033 | 3.60 |
| default, L1-D write-back | 168,703 | 1.24 |

Talking point: fib is recursive, so every call spills registers to the stack.
With a write-through L1-D and no write buffer, every one of those stores waits
for L2; switching the L1-D to write-back removes almost all of that (2.9× faster).

### 3.2 A bigger run (optional)

```bash
time $S -e qsort.elf -c ../configs/01_ideal.json            # ~2.5 M instructions, ~0.5 s
time $S -e qsort.elf -c ../configs/07_full_hierarchy.json   # ~2 s
time $S -e radix.elf -c ../configs/01_ideal.json            # ~2.0 M instructions, ~0.4 s
time $S -e radix.elf -c ../configs/07_full_hierarchy.json   # ~2.6 s
```

Avoid `05_no_cache.json` with the sorts: every access pays 200 cycles and it runs for minutes.

---

## 4. Configurations

All files in `configs/` are copies of `essentials/template.json` with a few keys changed:

| File | Change from the default |
|---|---|
| `01_ideal.json` | `CACHE_LEVELS: -1` (ideal memory, every access 1 cycle) |
| `02_ideal_no_forwarding.json` | ideal + `FORWARDING_SWITCH: 0` |
| `03_ideal_always_taken.json` | ideal + `BRANCH_PREDICTION_STRATEGY: 1` |
| `04_ideal_btfn.json` | ideal + `BRANCH_PREDICTION_STRATEGY: 2` |
| `05_no_cache.json` | `CACHE_LEVELS: 0` (straight to 200-cycle DRAM) |
| `06_l1_only.json` | `CACHE_LEVELS: 1` |
| `07_full_hierarchy.json` | none: the default L1-I/L1-D 2 KB, L2 8 KB, L3 16 KB |
| `08_full_hierarchy_writeback.json` | L1-D `write_policy: 1` (write-back) |
| `09_tiny_l1d.json` | L1-D 64 bytes, direct-mapped |
| `10_asm_memdump.json` | ideal + `DRAM_SIZE: 1048576` (1 MB, small memory dump) |

Key meanings, for live editing:

| Key | Values |
|---|---|
| `CACHE_LEVELS` | -1 ideal, 0 none, 1–3 levels |
| `FORWARDING_SWITCH` | 1 on, 0 off |
| `BRANCH_PREDICTION_STRATEGY` | 0 not-taken, 1 taken, 2 BTFN |
| `write_policy` | 0 write-through, 1 write-back |
| `write_miss_policy` | 1 write-allocate, 0 no-write-allocate |
| `replacement_policy` | 0 FIFO, 1 LRU |
| `cache_size`, `block_size` | bytes |
| `associativity` | ways (1 = direct-mapped) |
| `read_latency`, `write_latency`, `DRAM_*_LATENCY` | cycles |

A nice live trick: open `09_tiny_l1d.json`, change `cache_size` from 64 to 256
or 512, rerun `array.obj`, and watch the read misses drop to 0.

---

## 5. Clean up

```bash
cd .. && rm -rf run
```
