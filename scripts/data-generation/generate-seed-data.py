import argparse
import csv
import sys

from pathlib import Path

from key_generator import KeyFactory, SequentialIntKeyGenerator
from value_generator import ValueFactory, RandomIntegerGenerator

parser = argparse.ArgumentParser(
    prog="Seed Data Generation script for ORTOA-tee",
    description="Script to generate some seed data the ortoa-tee project"
)

parser.add_argument('-o', '--output_file', type=Path, default=Path("seed_data.csv"), help="File into which to write the sample seed data.")
parser.add_argument('-n', '--n_data_points', type=int, default=1000, help="Number of data points to generate.")

def main(argv):
    args = parser.parse_args(argv)

    num_data_points: int = args.n_data_points
    if not num_data_points > 0:
        raise ValueError(f"Expected n_data_points > 0. Received {num_data_points=}")

    output_file: Path = args.output_file
    if not output_file.suffix == ".csv":
        raise TypeError("Please create a file with extension .csv")

    key_generator: KeyFactory = SequentialIntKeyGenerator(start_key=1)
    value_generator: ValueFactory = RandomIntegerGenerator(min_val=1, max_val=1000000)
    
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)

        for _ in range(num_data_points):
            writer.writerow(["PUT", key_generator.generate_key(), value_generator.generate_value()])

    print(f"Data Generation Complete. Wrote data to file {output_file}")


if __name__ == "__main__":
	main(sys.argv[1:])
    