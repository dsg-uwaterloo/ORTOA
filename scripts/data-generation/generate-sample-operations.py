import argparse
import csv
import sys
import random

from enum import Enum
from pathlib import Path

from generators.value_generator import ValueFactory, RandomIntegerGenerator

parser = argparse.ArgumentParser(
    prog="Operation Generation script for ORTOA-tee",
    description="Script to generate some operations from a seed file"
)

parser.add_argument('-i', '--input_file', type=Path, required=True, help="Input file. This should be a Path to a csv of generated seed data.")
parser.add_argument('-o', '--output_file', type=Path, default=Path("sample_operations.csv"), help="File into which to write the operations.")
parser.add_argument('-n', '--n_operations', type=int, default=100, help="Number of operations to generate.")
parser.add_argument('-p', '--p_get', type=float, default=0.5, help="Probability of a GET request. 1-p_get = p_put (probability of a PUT request).")


# TODO: Fix the comment saying what input csv format is expected
"""
This is expecting a csv in the format specified by the DATA.md
"""
def get_keys_from_csv(input_file: Path) -> set:
    res = set()
    with open(input_file, 'r') as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            key = row[1]
            res.add(key)

    return res


class Operation(str, Enum):
    GET = "GET"
    PUT = "PUT"

def get_random_op(p_get: float) -> Operation:
    random_val = random.uniform(0, 1) # generate random value in range [0, 1]

    if 0 <= random_val <= p_get:
        return Operation.GET
    else:
        return Operation.PUT

    

def main(argv):
    args = parser.parse_args(argv)

    num_operations = args.n_operations
    if not num_operations > 0:
        raise ValueError(f"Expected n_operations > 0. Received {num_operations=}")

    input_file: Path = args.input_file
    if not input_file.exists():
        raise FileNotFoundError(f"Input file {input_file} was not found.")

    output_file: Path = args.output_file
    if not output_file.suffix == ".csv":
        raise ValueError("Please specify a file with extension .csv for the output file")
    
    p_get: float = args.p_get
    if not 0 <= p_get <= 1:
         raise ValueError(f"p_get must be in the range [0, 1]. Got {p_get=}")

    keys = list(get_keys_from_csv(input_file))
    value_generator: ValueFactory = RandomIntegerGenerator(min_val=1, max_val=1000000)

    with open(output_file, 'w') as csvfile:
        writer = csv.writer(csvfile)

        for _ in range(num_operations):
            op: Operation = get_random_op(p_get)

            if op == Operation.GET:
                writer.writerow(["GET", random.choice(keys)])
            elif op == Operation.PUT:
                writer.writerow(["PUT", random.choice(keys), value_generator.generate_value()])
            else:
                raise NotImplementedError("Unsupported Operation in operation generation.")
    
    print(f"Operation Generation Complete. Write data to file {output_file}")

if __name__ == "__main__":
	main(sys.argv[1:])
