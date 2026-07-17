import random
import argparse

parser = argparse.ArgumentParser(
    description="Generate a C-style array of random unsigned integers."
)

parser.add_argument("N", type=int, help="Number of elements to generate")
parser.add_argument("min", nargs="?", type=int, default=0,
                    help="Minimum value (default: 0)")
parser.add_argument("max", nargs="?", type=int, default=999999,
                    help="Maximum value (default: 999999)")

args = parser.parse_args()

if args.min > args.max:
    parser.error("min cannot be greater than max")

arr = [random.randint(args.min, args.max) for _ in range(args.N)]

print("{")
for i in range(0, args.N, 16):
    print("    " + ", ".join(map(str, arr[i:i+16])) + ",")
print("};")
