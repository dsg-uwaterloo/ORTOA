import argparse
from argparse import ArgumentParser
from datetime import datetime
from pathlib import Path
from typing import List

from ortoa.benchmark.infrastucture.experiment_collection import (
    ExperimentPath,
    collect_experiments,
)
from ortoa.benchmark.infrastucture.jobs import ClientJob, make_jobs
from ortoa.benchmark.infrastucture.runner import JobOrchestration
from ortoa.benchmark.infrastucture.stats_collection import Stats
from ortoa.benchmark.interface.experiment import (
    AtomicExperiment,
    Experiment,
    atomicize_experiments,
    load_experiments,
)


def parse_args() -> argparse.Namespace:
    parser = ArgumentParser()

    experiment_group = parser.add_argument_group(
        "Experiments", "Options to control experiments selected for compilation"
    )
    experiment_group.add_argument(
        "-e",
        "--experiments",
        nargs="+",
        type=Path,
        default=[],
        help="List of experiments to compile (experiment name should match zoo object)",
    )
    experiment_group.add_argument(
        "-d",
        "--experiment-dirs",
        nargs="+",
        type=Path,
        default=[],
        help="List of local directories to use for experiment files",
    )

    parser.add_argument(
        "-w",
        "--working-dir",
        type=Path,
        default=Path("out")
        / f"benchmark-{datetime.now().strftime('%Y-%m-%d-%H-%M-%S')}",
        required=False,
        help="Directory to use as base for experiment directory tree (default: %(default)s)",
    )

    args = parser.parse_args()

    if len(args.experiments + args.experiment_dirs) == 0:
        parser.error(
            "one of the arguments -e/--experiments or -d/--experiment-dirs is required"
        )

    return args


def benchmark(
    experiment_base: Path,
    experiment_names: List[Path],
) -> Stats:
    """Main entrypoint to the benchmarking flow"""

    # Get a path to every experiment file and verify the paths
    experiment_paths: List[ExperimentPath] = collect_experiments(experiment_names)

    # Load the experiments from yalm into the Experiment dataclass
    experiments: List[Experiment] = load_experiments(
        experiment_paths, base_dir=experiment_base
    )

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
    )
    results = orchestration.run_sequential()

    # Generate and return the statistics from the run
    return Stats.from_results(results)


def main():
    args = parse_args()
    args.working_dir.mkdir(parents=True, exist_ok=True)

    stats: Stats = benchmark(args.working_dir, args.experiments + args.experiment_dirs)

    stats.save_to(args.working_dir)


if __name__ == "__main__":
    main()
