#!/usr/bin/env python3

import argparse
import math
from collections import defaultdict

import numpy as np
import matplotlib.pyplot as plt


# -------------------------------------------------------
# Parse trace
# -------------------------------------------------------

def parse_trace(trace_file, bin_size):

    instr = defaultdict(int)
    reads = defaultdict(int)
    writes = defaultdict(int)

    min_bin = None
    max_bin = None

    with open(trace_file) as f:

        for line in f:

            line = line.strip()

            if not line:
                continue

            typ, addr = line.split()

            addr = int(addr, 16)

            b = addr // bin_size

            if min_bin is None:
                min_bin = b
                max_bin = b
            else:
                min_bin = min(min_bin, b)
                max_bin = max(max_bin, b)

            if typ == "i":
                instr[b] += 1

            elif typ == "r":
                reads[b] += 1

            elif typ == "w":
                writes[b] += 1

    return instr, reads, writes, min_bin, max_bin


# -------------------------------------------------------
# Normalize using logarithm
# -------------------------------------------------------

def normalize(counter):

    if len(counter) == 0:
        return {}

    values = {}

    maximum = max(math.log(v + 1) for v in counter.values())

    for k, v in counter.items():

        values[k] = math.log(v + 1) / maximum

    return values


# -------------------------------------------------------
# Build image
# -------------------------------------------------------

def make_image(instr, reads, writes,
               min_bin,
               max_bin,
               width):

    instr = normalize(instr)
    reads = normalize(reads)
    writes = normalize(writes)

    total_bins = max_bin - min_bin + 1

    height = math.ceil(total_bins / width)

    image = np.zeros((height, width, 3), dtype=float)

    for b in range(min_bin, max_bin + 1):

        idx = b - min_bin

        y = idx // width
        x = idx % width

        image[y, x, 0] = writes.get(b, 0.0)
        image[y, x, 1] = reads.get(b, 0.0)
        image[y, x, 2] = instr.get(b, 0.0)

    return image


# -------------------------------------------------------
# Plot
# -------------------------------------------------------

def plot(image,
         min_bin,
         max_bin,
         bin_size,
         width):

    fig, ax = plt.subplots(figsize=(14,10))

    ax.imshow(
        image,
        interpolation="nearest",
        origin="lower",
        aspect="auto"
    )

    ax.set_title("Memory Access Heatmap", fontsize=18)

    ax.set_xlabel("Memory Bin")

    ax.set_ylabel("Memory")

    height = image.shape[0]

    yticks = np.linspace(0, height-1, 8).astype(int)

    labels = []

    for y in yticks:

        b = min_bin + y * width

        labels.append(hex(b * bin_size))

    ax.set_yticks(yticks)

    ax.set_yticklabels(labels)

    ax.set_xticks([])

    from matplotlib.patches import Patch

    legend = [
        Patch(color="red", label="Writes"),
        Patch(color="green", label="Reads"),
        Patch(color="blue", label="Instruction Reads"),
    ]

    ax.legend(
        handles=legend,
        loc="upper right"
    )

    plt.tight_layout()

    plt.show()


# -------------------------------------------------------
# Main
# -------------------------------------------------------

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
        "-w",
        "--width",
        type=int,
        default=256,
        help="Number of bins per row."
    )

    args = parser.parse_args()

    instr, reads, writes, min_bin, max_bin = parse_trace(
        args.trace,
        args.bin_size
    )

    image = make_image(
        instr,
        reads,
        writes,
        min_bin,
        max_bin,
        args.width
    )

    plot(
        image,
        min_bin,
        max_bin,
        args.bin_size,
        args.width
    )


if __name__ == "__main__":
    main()
