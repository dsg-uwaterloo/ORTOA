from pathlib import Path
from pydantic import BaseModel
from typing import List

from extras.benchmark.interface.experiment import Experiment


class ExperimentParameters:
    ...


class ClientJob(BaseModel):
    """
    Job for testing and benchmarking the client. Satisfies runner.JobProtocol
    """

    directory: Path
    experiment_parameters: ExperimentParameters
    client_flags: str
    host_flags: str

    @property
    def name(self) -> str:
        return f"Job {self.experiment.name}"

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
        Setup the environment (flush & seed the database), then run the client operations
        """
        self.directory.mkdir(parents=True, exist_ok=False)

        self._flush_db()
        self._seed_db()
        self._perform_operations()
        self._save_results()
        self._flush_db()


def make_jobs(experiment_root: Path, experiments: List[Experiment]) -> List[ClientJob]:
    jobs: List[ClientJob] = []
    raise NotImplementedError
