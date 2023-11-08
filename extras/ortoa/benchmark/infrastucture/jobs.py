from pathlib import Path
from pydantic import BaseModel, Field
from typing import Any, List, ClassVar

import redis
import subprocess

from ortoa.benchmark.interface.experiment import AtomicExperiment, ExperimentMetatadata


class ClientFlags(BaseModel):
    initdb: bool = True
    nthreads: int = 1
    seed: Path = Field(required=True)
    operations: Path = Field(required=True)
    output: Path = Field(required=True)

    @property
    def initdb_flags(self) -> str:
        return f"--initdb --seed {self.seed} --nthreads {self.nthreads}"

    @property
    def operation_flags(self) -> str:
        return f"--seed {self.operations} --nthreads {self.nthreads} --output {self.output}"

    def model_post_init(self, __context: Any) -> None:
        return super().model_post_init(__context)


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

    rd: ClassVar[redis.Redis] = redis.Redis(host="localhost", port=6379)

    def __str__(self) -> str:
        return self.name

    def _flush_db(self) -> None:
        """Flush (empty) the database"""
        self.rd.flushdb(asynchronous=False)

    def _seed_db(self) -> None:
        """Seed the database based on seed file linked in experiment"""
        seed_command = [
            "./build/src/client/client"
        ] + self.client_flags.initdb_flags.split()
        subprocess.run(seed_command)

    def _perform_operations(self) -> None:
        """Perform operations based on file linked in experiment"""
        operations_command = [
            "./build/src/client/client"
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

        host_command = [
            "./build/src/host/ortoa-host",
            "./build/src/enclave/ortoa-enc.signed",
            "--simulate",
        ] + str(self.host_flags).split()
        with subprocess.Popen(host_command) as host_proc:
            self._flush_db()
            self._seed_db()
            self._perform_operations()
            self._flush_db()
            host_proc.terminate()

        self._save_results()


def make_jobs(
    experiment_root: Path, experiments: List[AtomicExperiment]
) -> List[ClientJob]:
    jobs: List[ClientJob] = []

    for experiment in experiments:
        e_client_flags = ClientFlags(
            seed=experiment.seed_data,
            operations=experiment.operations,
            output=experiment.output_directory / "results.txt",
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
