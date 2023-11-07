from pathlib import Path
from pydantic import BaseModel, Field
from typing import List, ClassVar

import redis
import subprocess

from ortoa.benchmark.interface.experiment import AtomicExperiment, ExperimentMetatadata


class ClientFlags(BaseModel):
    initdb: bool = True
    seed: Path = Field(required=True)
    operations: Path = Field(required=True)
    nthreads: int = 1

    @property
    def initdb_flags(self) -> str:
        return f"--initdb --seed {self.seed} --nthreads {self.nthreads}"

    def operation_flags(self) -> str:
        return f"--seed {self.operations} --nthreads {self.nthreads}"


class HostFlags(BaseModel):
    nthreads: int = 1
    simulate: bool = True

    def __str__(self) -> str:
        return f"--nthreads {self.nthreads}"


class ClientJob(BaseModel):
    """
    Job for testing and benchmarking the client. Satisfies runner.JobProtocol
    """

    class Config:
        arbitrary_types_allowed = True

    name: str
    directory: Path
    metadata: ExperimentMetatadata

    seed_data: Path
    operations: Path

    client_flags: ClientFlags
    host_flags: HostFlags

    rd: ClassVar[redis.Redis] = redis.Redis(host="localhost", port=6397)

    def __str__(self) -> str:
        return self.name

    def _flush_db(self) -> None:
        """Flush (empty) the database"""
        self.rd.flushdb()

    def _seed_db(self) -> None:
        """Seed the database based on seed file linked in experiment"""
        seed_command = ["ortoa-client-run"] + self.client_flags.initdb_flags.split()
        subprocess.run(seed_command)

    def _perform_operations(self) -> None:
        """Perform operations based on file linked in experiment"""
        operations_command = [
            "ortoa-client-run"
        ] + self.client_flags.operation_flags.split()
        subprocess.run(operations_command)

    def _save_results(self) -> None:
        """Save the results of this job"""
        raise NotImplementedError

    def __call__(self) -> None:
        """
        Setup the environment (flush & seed the database), then run the client operations in self.directory
        """
        self.directory.mkdir(parents=True, exist_ok=False)

        subprocess.run(["source", "scripts/ortoa-lib.sh"])

        self._flush_db()

        host_command = ["ortoa-simulate"] + str(self.host_flags).split()
        with subprocess.Popen(host_command):
            self._seed_db()
            self._perform_operations()

        self._save_results()
        self._flush_db()


def make_jobs(
    experiment_root: Path, experiments: List[AtomicExperiment]
) -> List[ClientJob]:
    jobs: List[ClientJob] = []

    for experiment in experiments:
        e_client_flags = ClientFlags(
            seed=experiment.seed_data, operations=experiment.operations
        )

        for flag in experiment.client_flags:
            if flag.name == "nthreads":
                e_client_flags.nthreads = flag.value
            elif flag.name == "client_logging_enabled":
                pass
            else:
                raise ValueError("Client flag not recognized")

        e_host_flags = HostFlags()
        for flag in experiment.host_flags:
            if flag.name == "nthreads":
                e_host_flags.nthreads = flag.value
            elif flag.name == "host_logging_enabled":
                pass
            else:
                raise ValueError("Host flag not recognized")

        jobs.append(
            ClientJob(
                name=experiment.name,
                directory=experiment.output_directory,
                metadata=experiment.metadata,
                seed_data=experiment.seed_data,
                operations=experiment.operations,
                client_flags=e_client_flags,
                host_flags=e_host_flags,
            )
        )

    return jobs
