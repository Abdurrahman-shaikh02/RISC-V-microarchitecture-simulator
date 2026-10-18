#!/bin/bash
set -e
P=/home/abdurrahman/Documents/projects/compute_engine; E=/tmp/claude-1000/-home-abdurrahman-Documents-projects-compute-engine/9f28a0f1-109a-463b-8a91-0ec3c6c2fd96/scratchpad/exp
STD="$P/essentials/stdlib/__mulsi3.c $P/essentials/stdlib/__divsi3.c $P/essentials/stdlib/__udivsi3.c $P/essentials/stdlib/__umodsi3.c $P/essentials/memcpy.c"
RV="riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 -nostdlib -ffreestanding -O2"
for w in fib qsort radix matmul bubble; do
  $RV -T $P/essentials/linker.ld $P/essentials/crt0.S $E/src/$w.c $STD -o $E/bin/$w.elf
  $RV -T $P/essentials/spike/linker_spike.ld $E/src/crt0_spike.S $E/src/$w.c $STD -o $E/bin/$w.spike.elf
  gcc -O2 -Dmain=bench_main -c $E/src/$w.c -o $E/bin/$w.o && gcc $E/bin/$w.o $E/src/native_wrap.c -o $E/bin/$w.native
done
CM="$E/coremark"
CMSRC="$CM/core_main.c $CM/core_list_join.c $CM/core_matrix.c $CM/core_state.c $CM/core_util.c $CM/barebones/core_portme.c $CM/barebones/ee_printf.c"
CMF=(-DITERATIONS=${CM_ITERS:-10} -DTOTAL_DATA_SIZE=2000 "-DFLAGS_STR=\"-O2\"" -I$CM -I$CM/barebones)
$RV "${CMF[@]}" -T $P/essentials/linker.ld $P/essentials/crt0.S $CMSRC $STD -o $E/bin/coremark.elf
$RV "${CMF[@]}" -T $P/essentials/spike/linker_spike.ld $E/src/crt0_spike.S $CMSRC $STD -o $E/bin/coremark.spike.elf
gcc -O2 -w "${CMF[@]}" -Dmain=bench_main $CMSRC $E/src/native_wrap.c -o $E/bin/coremark.native
