import argparse
import datetime
from argparse import ArgumentParser
from pathlib import Path
from typing import List, Optional, Union

from ortoa.benchmark.infrastucture.experiment_collection import (
    collect_experiments,
    ExperimentPath,
)
from ortoa.benchmark.infrastucture.jobs import ClientJob, make_jobs
from ortoa.benchmark.interface.experiment import (
    Experiment,
    load_experiments,
    atomicize_experiments,
    AtomicExperiment,
)
from ortoa.benchmark.infrastucture.runner import JobOrchestration


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
        nargs="+",
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
    experiment_names: List[Path],
    max_processes: Optional[int] = None,
    log_errors_in_main_thread: bool = False,
) -> Stats:
    """Main entrypoint to the benchmarking flow

    Args:
        experiment_base (Path): _description_
        experiment_names (List[Path]): _description_
        max_processes (Optional[int], optional): _description_. Defaults to None.
        log_errors_in_main_thread (bool, optional): _description_. Defaults to False.

    Returns:
        Stats: _description_
    """
    # Get a path to every experiment file and verify the paths
    experiment_paths: List[ExperimentPath] = collect_experiments(experiment_names)

    # Load the experiments from yalm into the Experiment dataclass
    experiments: List[Experiment] = load_experiments(experiment_paths)

    # Generate data for the experiments that require it
    for experiment in experiments:
        experiment.generate_data()

    # Convert the experiments into a list of atomic (fully reduced) experiments
    atomic_experiments: List[AtomicExperiment] = atomicize_experiments(experiments)

    # Create the jobs from the experiments
    jobs: List[ClientJob] = make_jobs(
        experiment_root=experiment_base, experiments=atomic_experiments
    )

    # Orchestrate and the jobs
    orchestration = JobOrchestration(
        jobs=jobs,
        max_processes=max_processes,
        log_errors_in_main_thread=log_errors_in_main_thread,
    )
    results = orchestration.run_sequential()

    # Generate and return the statistics from the run
    return Stats.from_results(results)


def main():
    args = parse_args()
    args.working_dir.mkdir(parents=True, exist_py=False)

    stats: Stats = benchmark(
        args.working_dir, args.experiments + args.experiments_dirs, args.max_processes
    )

    # TODO: Save the stats somewhere

    # TODO: create the HTML


if __name__ == "__main__":
    main()
