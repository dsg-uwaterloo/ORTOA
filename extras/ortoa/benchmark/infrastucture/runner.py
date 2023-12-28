from pathlib import Path
from typing import (
    Any,
    Generic,
    List,
    Optional,
    Protocol,
    Sequence,
    TypeVar,
    runtime_checkable,
)

from alive_progress import alive_bar, alive_it
from pydantic import BaseModel


@runtime_checkable
class JobProtocol(Protocol):
    """
    Protocol class (https://peps.python.org/pep-0544/) providing template for jobs runnable by the JobOrchestration class.
    To satisfy this protocol, a class must have all the same members and methods (but can have more). A satisfying class
    does not have to inherit from the protocol. Other examples of protocols include typing.Sequence
    """

    directory: Path

    @property
    def name(self) -> str:
        """Name of the job, used for logging output"""
        raise NotImplementedError

    def __str__(self) -> str:
        raise NotImplementedError

    def __call__(self) -> None:
        """Execute the job"""
        raise NotImplementedError


# JobT binds to any class satisfying JobProtocol
JobT = TypeVar("JobT", bound=JobProtocol)


class Result(BaseModel, Generic[JobT]):
    """
    Result[Job] is a Job and an exception
    """

    class Config:
        arbitrary_types_allowed = True

    job: JobT
    result_path: Path
    exception: Optional[BaseException]


class JobOrchestration(BaseModel, Generic[JobT]):
    """
    Given a sequence of jobs, schedule the jobs in a process pool, managing job cancellation and progress reporting
    """

    class Config:
        arbitrary_types_allowed = True

    jobs: Sequence[JobT]
    max_processes: Optional[int]
    log_errors_in_main_thread: bool = False

    def model_post_init(self, __context: Any) -> None:
        if len(self.jobs) == 0:
            raise ValueError(f"{__class__} configured with no jobs")
        return super().model_post_init(__context)

    def run(self) -> List[Result[JobT]]:
        """Leaving this for when I'm ready to implement multiprocessing for the benchmarking"""
        raise NotImplementedError

    def run_sequential(self) -> List[Result[JobT]]:
        results: List[Result[JobT]] = []

        with alive_bar(len(self.jobs)) as bar:
            bar.text("Starting the benchmark!")

            for job in self.jobs:
                bar.text(f"Running Job: {str(job)}")  # update progress bar

                job()  # run the job

                # save the results
                results.append(
                    Result(job=job, result_path=job.client_flags.output, exception=None)
                )

                bar()  # increment the progress bar status

            bar.text("Benchmark complete!")

        return results
