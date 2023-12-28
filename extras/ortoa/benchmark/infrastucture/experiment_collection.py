import itertools
from pathlib import Path
from typing import Any, Iterable, List

from pydantic import BaseModel
from typing_extensions import Self


class ExperimentPath(BaseModel):
    """
    Experiment and associated files as an object on local filesystem
    """

    experiment_path: Path

    def model_post_init(self, __context: Any) -> None:
        assert self.experiment_path.is_file()
        return super().model_post_init(__context)


    @classmethod
    def construct_experiments(cls, experiment: Path) -> List[Self]:
        """
        Construct an list of ExperimentPath instances
        """

        if experiment.is_file():
            return cls.from_path(experiment)

        elif experiment.is_dir():
            return cls.from_dir(experiment)

        raise TypeError

    @classmethod
    def from_path(cls, experiment: Path) -> List[Self]:
        return [cls(experiment_path=experiment)]

    @classmethod
    def from_dir(cls, experiment_dir: Path) -> List[Self]:
        return [
            cls(experiment_path=e) for e in experiment_dir.glob("**/*.yaml")
        ]


def collect_experiments(experiments: Iterable[Path]) -> List[ExperimentPath]:
    """
    Given a list of experiment names, find the experiments and collect them into an interable
    """
    return list(
        itertools.chain.from_iterable(
            [ExperimentPath.construct_experiments(experiment) for experiment in experiments]
        )
    )
