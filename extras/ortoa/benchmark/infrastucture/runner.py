from pathlib import Path
from typing import (
    Any,
    Callable,
    Dict,
    Generic,
    List,
    Optional,
    Protocol,
    Sequence,
    TypeVar,
    Union,
    runtime_checkable,
)

from pydantic import BaseModel, Field
from typing_extensions import Self

from icecream import ic


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
            raise ValueError(
                f"{__class__} configured with no jobs"
            )  # TODO: Move to logger
        return super().model_post_init(__context)

    def run(self) -> List[Result[JobT]]:
        """Leaving this for when I'm ready to implement multithreading for the benchmarking"""
        assert self.max_processes >= 1
        raise NotImplementedError

    def run_sequential(self) -> List[Result[JobT]]:
        for job in self.jobs:
            job.directory.mkdir(parents=True)
            with open(job.directory / "output.yaml", "w") as f:
                f.write("meta: Output for job " + job.name)

        raise NotImplementedError
        return []
