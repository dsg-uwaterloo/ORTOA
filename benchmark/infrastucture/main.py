import argparse
from argparse import ArgumentParser
import datetime
from pathlib import Path
from typing import List, Union, Optional
from benchmark.infrastucture.experiment_collection import collect_experiments


class Stats:
    ...


def parse_args() -> argparse.Namespace:
    parser = ArgumentParser()

    experiment_group = parser.add_argument_group(
        "Experiments", "Options to control experiments selected for compilation"
    )
    experiment_group.add_argument(
        "-e",
        "--experiments",
        nargs="+",
        type=str,
        default=[],
        required=True,
        help="List of experiments to compile (experiment name should match zoo object)",
    )
    experiment_group.add_argument(
        "-d",
        "--experiment-dirs",
        narg="+",
        type=Path,
        default=[],
        required=False,
        help="List of local directories to use for experiment files",
    )

    parser.add_argument(
        "-w",
        "--working-dir",
        type=Path,
        default=Path.cwd() / f"benchmark-{datetime.date.today()}",
        required=False,
        help="Directory to use as base for experiment directory tree (default: %(default)s)",
    )
    parser.add_argument(
        "-m",
        "--max-processes",
        type=int,
        default=None,
        required=False,
        help="Maximum number of processes to use when running experiments (default: %(default)s)",
    )

    args = parser.parse_args()

    if len(args.networks + args.network_dirs) == 0:
        parser.error(
            "one of the arguments -e/--experiments or -d/--experiment-dirs is required"
        )

    return args


def benchmark(
    experiment_base: Path,
    experiment_names: List[Union[str, Path]],
    max_processes: Optional[int] = None,
    log_errors_in_main_thread: bool = False,
) -> Stats:
    """Main entrypoint to benchmarking flow

    Args:
        experiment_base (Path): Directory to use as base for experiment directory tree
        experiment_names (List[str]): _description_
        max_processes (Optional[int], optional): _description_. Defaults to None.

    Returns:
        Stats: _description_
    """

    # Get a path to every experiment file and verify the paths
    experiments = collect_experiments(experiment_names)

    # Create the jobs from the experiments
    jobs: List[Job] = make_jobs(experiment_base, experiments)

    # Orchestrate and the jobs
    orchestration = JobOrchestration(jobs, max_processes, log_errors_in_main_thread)
    results = orchestration.run()

    # Generate and return the statistics from the run
    return Stats.from_results(results)


def main():
    args = parse_args()
    args.working_dir.mkdir(parents=True, exist_py=False)

    stats = benchmark(
        args.working_dir, args.experiments + args.experiments_dirs, args.max_processes
    )

    # TODO: Save the stats somewhere

    # TODO: create the HTML


if __name__ == "__main__":
    main()
