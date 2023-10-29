from pathlib import Path
from typing import Any, Iterable, List, Union

from pydantic import BaseModel
from typing_extensions import Self


def collect_experiments(
    experiments: Iterable[Union[str, Path]]
) -> List["ExperimentPaths"]:
    """
    Given a list of experiment names, find the experiments and collect them into an interable
    """
    return [ExperimentPaths.construct(experiment) for experiment in experiments]


class ExperimentPaths(BaseModel):
    """
    Experiment and associated files as an object on local filesystem
    """

    experiment_path: Path

    def model_post_init(self, __context: Any) -> None:
        # TODO: verify that all paths are files
        return super().model_post_init(__context)

    @classmethod
    def construct(cls, experiment: Union[str, Path]) -> Self:
        """
        Construct an instance of ExperimentPaths
        """

        if isinstance(experiment, str):
            return cls.from_zoo(experiment)
        elif isinstance(experiment, Path):
            return cls.from_path(experiment)

        raise TypeError(
            f"Unsupported argument to {__class__}.construct: {experiment} of type {type(experiment).__name__}"
        )

    @classmethod
    def from_path(cls, Path) -> Self:
        return
