import argparse
from argparse import ArgumentParser

import csv
import random

from enum import Enum
from pathlib import Path

from generators.value_generator import ValueFactory, RandomIntegerGenerator


def parse_args() -> argparse.Namespace:
    parser = ArgumentParser(
        prog="Operation Generation script for ORTOA-tee",
        description="Script to generate some operations from a seed file",
    )

    parser.add_argument(
        "-i",
        "--input_file",
        type=Path,
        required=True,
        help="Input file. This should be a Path to a csv of generated seed data.",
    )
    parser.add_argument(
        "-o",
        "--output_file",
        type=Path,
        default=Path("sample_operations.csv"),
        help="File into which to write the operations.",
    )
    parser.add_argument(
        "-n",
        "--n_operations",
        type=int,
        default=100,
        help="Number of operations to generate.",
    )
    parser.add_argument(
        "-p",
        "--p_get",
        type=float,
        default=0.5,
        help="Probability of a GET request. 1-p_get = p_put (probability of a PUT request).",
    )

    args = parser.parse_args()

    if not args.input_file.exists():
        raise FileNotFoundError(f"Input file {args.input_file} was not found.")

    if not args.output_file.suffix == ".csv":
        raise ValueError(
            "Please specify a file with extension .csv for the output file"
        )

    if not 0 <= args.p_get <= 1:
        raise ValueError(f"p_get must be in the range [0, 1]. Got {args.p_get=}")

    return args


def get_keys_from_csv(input_file: Path) -> set:
    res = set()
    with open(input_file, "r") as csvfile:
        reader = csv.reader(csvfile, delimiter=" ")
        for row in reader:
            key = row[1]
            res.add(key)

    return res


class Operation(str, Enum):
    GET = "GET"
    PUT = "PUT"


def get_random_op(p_get: float) -> Operation:
    random_val = random.uniform(0, 1)  # generate random value in range [0, 1]

    if 0 <= random_val <= p_get:
        return Operation.GET
    else:
        return Operation.PUT


def generate_operations(
    num_operations: int,
    input_file: Path,
    output_file: Path,
    p_get: float,
    value_generator: ValueFactory,
) -> None:
    keys = list(get_keys_from_csv(input_file))

    with open(output_file, "w") as csvfile:
        writer = csv.writer(csvfile, delimiter=" ")

        for _ in range(num_operations):
            op: Operation = get_random_op(p_get)

            if op == Operation.GET:
                writer.writerow(["GET", random.choice(keys)])
            elif op == Operation.PUT:
                writer.writerow(
                    ["PUT", random.choice(keys), value_generator.generate_value()]
                )
            else:
                raise NotImplementedError(
                    "Unsupported Operation in operation generation."
                )


def main():
    args = parse_args()

    value_generator: ValueFactory = RandomIntegerGenerator(min_val=1, max_val=99999)

    generate_operations(
        num_operations=args.n_operations,
        input_file=args.input_file,
        output_file=args.output_file,
        p_get=args.p_get,
        value_generator=value_generator,
    )

    print(f"Operation Generation Complete. Write data to file {args.output_file}")


if __name__ == "__main__":
    main()
