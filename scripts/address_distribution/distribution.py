#!/usr/bin/env python3

import argparse
from collections import defaultdict

import matplotlib.pyplot as plt


# -----------------------------------------------------------
# Read trace
# -----------------------------------------------------------

def parse_trace(filename, bin_size):

    instr = defaultdict(int)
    reads = defaultdict(int)
    writes = defaultdict(int)

    with open(filename) as f:

        for line in f:

            line = line.strip()

            if not line:
                continue

            typ, addr = line.split()

            addr = int(addr, 16)

            bin_addr = (addr // bin_size) * bin_size

            if typ == 'i':
                instr[bin_addr] += 1

            elif typ == 'r':
                reads[bin_addr] += 1

            elif typ == 'w':
                writes[bin_addr] += 1

    return instr, reads, writes


# -----------------------------------------------------------
# Prepare arrays
# -----------------------------------------------------------

def prepare(instr, reads, writes):

    bins = sorted(
        set(instr.keys()) |
        set(reads.keys()) |
        set(writes.keys())
    )

    x = list(range(len(bins)))

    instr_y = [instr.get(b, 0) for b in bins]
    read_y = [reads.get(b, 0) for b in bins]
    write_y = [writes.get(b, 0) for b in bins]

    return bins, x, instr_y, read_y, write_y


# -----------------------------------------------------------
# Plot
# -----------------------------------------------------------

def plot_bars(bins, x, instr, reads, writes, bin_size):

    width = 0.28

    plt.figure(figsize=(18,7))

    plt.bar(
        [i-width for i in x],
        instr,
        width=width,
        alpha=0.7,
        label="Instruction Reads"
    )

    plt.bar(
        x,
        reads,
        width=width,
        alpha=0.7,
        label="Data Reads"
    )

    plt.bar(
        [i+width for i in x],
        writes,
        width=width,
        alpha=0.7,
        label="Data Writes"
    )

    finish_plot(bins, x, bin_size)


def plot_scatter(bins, x, instr, reads, writes, bin_size):

    plt.figure(figsize=(18,7))

    plt.scatter(x, instr, s=8, label="Instruction Reads")

    plt.scatter(x, reads, s=8, label="Data Reads")

    plt.scatter(x, writes, s=8, label="Data Writes")

    finish_plot(bins, x, bin_size)


def plot_stem(bins, x, instr, reads, writes, bin_size):

    plt.figure(figsize=(18,7))

    plt.stem(
        x,
        instr,
        linefmt='C0-',
        markerfmt='C0o',
        basefmt=' '
    )

    plt.stem(
        x,
        reads,
        linefmt='C1-',
        markerfmt='C1o',
        basefmt=' '
    )

    plt.stem(
        x,
        writes,
        linefmt='C2-',
        markerfmt='C2o',
        basefmt=' '
    )

    plt.plot([], [], color='C0', label='Instruction Reads')
    plt.plot([], [], color='C1', label='Data Reads')
    plt.plot([], [], color='C2', label='Data Writes')

    finish_plot(bins, x, bin_size)


# -----------------------------------------------------------
# Common formatting
# -----------------------------------------------------------

def finish_plot(bins, x, bin_size):

    tick_step = max(1, len(bins)//10)

    plt.xticks(
        x[::tick_step],
        [hex(i) for i in bins[::tick_step]],
        rotation=45
    )

    plt.xlabel(f"Memory Bin ({bin_size} Bytes)")
    plt.ylabel("Access Count")
    plt.title("Memory Access Distribution")

    plt.grid(alpha=0.3)
    plt.legend()

    plt.tight_layout()

    plt.show()


# -----------------------------------------------------------
# Main
# -----------------------------------------------------------

def main():

    parser = argparse.ArgumentParser()

    parser.add_argument("trace")

    parser.add_argument(
        "-b",
        "--bin-size",
        type=lambda x: int(x,0),
        default=64
    )

    parser.add_argument(
        "-p",
        "--plot",
        choices=[
            "bars",
            "scatter",
            "stem"
        ],
        default="bars"
    )

    args = parser.parse_args()

    instr, reads, writes = parse_trace(
        args.trace,
        args.bin_size
    )

    bins, x, instr_y, read_y, write_y = prepare(
        instr,
        reads,
        writes
    )

    if args.plot == "bars":

        plot_bars(
            bins,
            x,
            instr_y,
            read_y,
            write_y,
            args.bin_size
        )

    elif args.plot == "scatter":

        plot_scatter(
            bins,
            x,
            instr_y,
            read_y,
            write_y,
            args.bin_size
        )

    elif args.plot == "stem":

        plot_stem(
            bins,
            x,
            instr_y,
            read_y,
            write_y,
            args.bin_size
        )


if __name__ == "__main__":
    main()
