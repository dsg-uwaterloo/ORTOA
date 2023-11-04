from pathlib import Path
from pydantic import BaseModel
from typing import List

from extras.benchmark.interface.experiment import (
    AtomicExperiment,
)

from extras.benchmark.interface.flags import AnnotatedClientFlag, AnnotatedHostFlag


class ClientJob(BaseModel):
    """
    Job for testing and benchmarking the client. Satisfies runner.JobProtocol
    """

    directory: Path
    metadata: ExperimentMetatadata

    seed_data: Path
    operations: Path

    client_flags: ClientFlags
    host_flags: HostFlags

    def __str__(self) -> str:
        return self.name()

    def _flush_db(self) -> None:
        """Flush (empty) the database"""
        raise NotImplementedError

    def _seed_db(self) -> None:
        """Seed the database based on seed file linked in experiment"""
        raise NotImplementedError

    def _perform_operations(self) -> None:
        """Perform operations based on file linked in experiment"""
        raise NotImplementedError

    def _save_results(self) -> None:
        """Save the results of this job"""
        raise NotImplementedError

    def __call__(self) -> None:
        """
        Setup the environment (flush & seed the database), then run the client operations in self.directory
        """
        self.directory.mkdir(parents=True, exist_ok=False)

        self._flush_db()
        self._seed_db()
        self._perform_operations()
        self._save_results()
        self._flush_db()


def make_jobs(
    experiment_root: Path, experiments: List[AtomicExperiment]
) -> List[ClientJob]:
    jobs: List[ClientJob] = []

    for experiment in experiments:
        pass  # TODO:

    raise NotImplementedError
