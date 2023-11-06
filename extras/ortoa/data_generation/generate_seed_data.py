import argparse
from argparse import ArgumentParser

import csv
from pathlib import Path

from ortoa.data_generation.generators.key_generator import (
    KeyFactory,
    SequentialIntKeyGenerator,
)
from ortoa.data_generation.generators.value_generator import (
    ValueFactory,
    RandomIntegerGenerator,
)


def parse_args() -> argparse.Namespace:
    parser = ArgumentParser(
        prog="Seed Data Generation script for ORTOA-tee",
        description="Script to generate some seed data the ortoa-tee project",
    )

    parser.add_argument(
        "-o",
        "--output_file",
        type=Path,
        default=Path("seed_data.csv"),
        help="File into which to write the sample seed data.",
    )

    parser.add_argument(
        "-n",
        "--n_data_points",
        type=int,
        default=1000,
        help="Number of data points to generate.",
    )

    args = parser.parse_args()
    return args


def generate_data(
    key_generator: KeyFactory,
    value_generator: ValueFactory,
    output_file: Path,
    num_data_points: int,
) -> None:
    with open(output_file, "w") as csvfile:
        writer = csv.writer(csvfile, delimiter=" ")

        for _ in range(num_data_points):
            writer.writerow(
                ["SET", key_generator.generate_key(), value_generator.generate_value()]
            )


def main():
    args = parse_args()

    num_data_points: int = args.n_data_points
    if not num_data_points > 0:
        raise ValueError(f"Expected n_data_points > 0. Received {num_data_points=}")

    output_file: Path = args.output_file
    if not output_file.suffix == ".csv":
        raise ValueError(
            "Please specify a file with extension .csv for the output file"
        )

    key_generator: KeyFactory = SequentialIntKeyGenerator(start_key=1)
    value_generator: ValueFactory = RandomIntegerGenerator(min_val=1, max_val=99999)

    generate_data(
        key_generator=key_generator,
        value_generator=value_generator,
        output_file=output_file,
        num_data_points=num_data_points,
    )

    print(f"Data Generation Complete. Wrote data to file {output_file}")


if __name__ == "__main__":
    main()
