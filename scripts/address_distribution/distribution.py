#!/usr/bin/env python3

import argparse
from collections import defaultdict
import matplotlib.pyplot as plt


def parse_trace(filename, bin_size):
    instr_bins = defaultdict(int)
    read_bins = defaultdict(int)
    write_bins = defaultdict(int)

    with open(filename, "r") as f:
        for line in f:
            line = line.strip()

            if not line:
                continue

            typ, addr = line.split()

            addr = int(addr, 16)
            bin_addr = (addr // bin_size) * bin_size

            if typ == "i":
                instr_bins[bin_addr] += 1
            elif typ == "r":
                read_bins[bin_addr] += 1
            elif typ == "w":
                write_bins[bin_addr] += 1

    return instr_bins, read_bins, write_bins


def plot(instr, reads, writes, bin_size):

    all_bins = sorted(set(instr.keys()) |
                      set(reads.keys()) |
                      set(writes.keys()))

    if not all_bins:
        print("Empty trace.")
        return

    x = range(len(all_bins))

    instr_y = [instr.get(b, 0) for b in all_bins]
    read_y = [reads.get(b, 0) for b in all_bins]
    write_y = [writes.get(b, 0) for b in all_bins]

    plt.figure(figsize=(18, 7))

    plt.plot(x, instr_y, label="Instruction Reads", linewidth=1.5)
    plt.plot(x, read_y, label="Data Reads", linewidth=1.5)
    plt.plot(x, write_y, label="Data Writes", linewidth=1.5)

    tick_step = max(1, len(all_bins) // 10)

    plt.xticks(
        range(0, len(all_bins), tick_step),
        [hex(all_bins[i]) for i in range(0, len(all_bins), tick_step)],
        rotation=45,
    )

    plt.xlabel(f"Memory Bin ({bin_size} Bytes)")
    plt.ylabel("Number of Accesses")
    plt.title("Memory Access Distribution")
    plt.grid(True, alpha=0.3)
    plt.legend()

    plt.tight_layout()
    plt.show()


def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "trace",
        help="Trace file"
    )

    parser.add_argument(
        "-b",
        "--bin-size",
        type=lambda x: int(x, 0),
        default=64,
        help="Bin size in bytes (default=64)"
    )

    args = parser.parse_args()

    instr, reads, writes = parse_trace(args.trace, args.bin_size)

    plot(instr, reads, writes, args.bin_size)


if __name__ == "__main__":
    main()
