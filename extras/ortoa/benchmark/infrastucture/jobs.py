import json
import os
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, ClassVar, List, Tuple

import redis
import yaml
from pydantic import BaseModel, Field

from ortoa.benchmark.interface.experiment import AtomicExperiment, ExperimentMetatadata

SLEEP_TIME = 3

@dataclass
class LogFiles:
    client_stdout: Path
    client_stderr: Path
    host_stdout: Path
    host_stderr: Path

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

    @property
    def seed_command(self) -> List[str]:
        return [
            "./install/bin/client"
        ] + self.client_flags.initdb_flags.split()
    
    @property
    def operations_command(self) -> List[str]:
        return [
            "./install/bin/client"
        ] + self.client_flags.operation_flags.split()


    @property
    def host_command(self) -> List[str]:
        return [
            "./install/bin/ortoa-host",
            "./build/src/enclave/ortoa-enc.signed",
            "--simulate",
        ] + str(self.host_flags).split()

    _rd: ClassVar[redis.Redis] = redis.Redis(host="20.85.122.103", port=6379)

    def __str__(self) -> str:
        return self.name

    def _flush_db(self) -> None:
        """Flush (empty) the database"""
        self._rd.flushdb(asynchronous=False)
    
    def _write_debug_scripts(self) -> None:
        """Write out shell scripts to rerun-client for easier debugging"""
        seed_script_path: Path = self.directory / "seed.sh"
        with seed_script_path.open("w") as seed_debug_script:
            seed_debug_script.write("#!/bin/bash\n")
            seed_debug_script.write(" ".join(self.seed_command) + "\n")
        os.chmod(seed_script_path, 0o755)
        
        operations_script_path: Path = self.directory / "operations.sh"
        with operations_script_path.open("w") as operations_debug_script:
            operations_debug_script.write("#!/bin/bash\n")
            operations_debug_script.write(" ".join(self.operations_command) + "\n")
        os.chmod(operations_script_path, 0o755)

    def _seed_db(self) -> None:
        """Seed the database based on seed file linked in experiment"""
        log_file_paths = self._get_log_file_paths()

        # stdout & stderr will be redirected to these files
        client_stdout = log_file_paths.client_stdout.open("w")
        client_stderr = log_file_paths.client_stderr.open("w")

        subprocess.run(self.seed_command, stdout=client_stdout, stderr=client_stderr)
        
        time.sleep(2)
        
        # close the files where logs were written
        client_stdout.close()
        client_stderr.close()

    def _perform_operations(self) -> None:
        """Perform operations based on file linked in experiment"""
        log_file_paths = self._get_log_file_paths()

        # stdout & stderr will be redirected to these files
        client_stdout = log_file_paths.client_stdout.open("a")
        client_stderr = log_file_paths.client_stderr.open("a")

        subprocess.run(self.operations_command, stdout=client_stdout, stderr=client_stderr)

        time.sleep(2)

        # close the files where logs were written
        client_stdout.close()
        client_stderr.close()
        

    def _save_results(self) -> None:
        """Save the results of this job"""
        config_dump_path = self.directory / "config.yaml"
        data = json.loads(self.model_dump_json())

        with config_dump_path.open("w") as f:
            yaml.safe_dump(data, f)
    
    def _cleanup(self) -> None:
        """Get rid of empty log files in the benchmarking output"""
        return
        def file_is_empty(file: Path) -> bool:
            return os.stat(file).st_size == 0

        fs = self._get_log_file_paths()

        # for file in fs.client_stdout, fs.client_stderr, fs.host_stdout, fs.host_stderr:
        #     if file_is_empty(file):
        #         file.unlink() # delete the file

    def _get_log_file_paths(self) -> LogFiles:
        return LogFiles(
            client_stdout = self.directory / "client_stdout.log",
            client_stderr = self.directory / "client_stderr.log",
            host_stdout = self.directory / "host_stdout.log",
            host_stderr = self.directory / "host_stderr.log"
        )

    def __call__(self) -> None:
        """
        Setup the environment (flush & seed the database), then run the client operations in self.directory
        """
        self.directory.mkdir(parents=True, exist_ok=False)
        
        # log_file_paths = self._get_log_file_paths()

        # stdout & stderr will be redirected to these files
        # host_stdout = log_file_paths.host_stdout.open("w")
        # host_stderr = log_file_paths.host_stderr.open("w")
    
        # with subprocess.Popen(self.host_command, stdout=host_stdout, stderr=host_stderr) as host_proc:
        self._write_debug_scripts()
        time.sleep(SLEEP_TIME)
        self._flush_db()
        time.sleep(SLEEP_TIME)
        self._seed_db()
        time.sleep(SLEEP_TIME)
        self._perform_operations()
        time.sleep(SLEEP_TIME)
        self._flush_db()
        time.sleep(SLEEP_TIME)
            # host_proc.terminate()

        # close the files where logs were written
        # host_stdout.close()
        # host_stderr.close()

        self._save_results()

        self._cleanup()


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
