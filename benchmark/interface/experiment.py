import yaml

from pathlib import Path
from typing import Any, Generic, List, Optional, TypeVar, Union

from pydantic import BaseModel, Field

from benchmark.interface.flags import AnnotatedClientFlag, AnnotatedHostFlag
from benchmark.infrastucture.experiment_collection import ExperimentPath

FlagT = TypeVar("FlagT", bound=Union[AnnotatedClientFlag, AnnotatedHostFlag])


class ClientData(BaseModel):
    seed: Optional[Path] = None
    operations: Optional[Path] = None


class Config(BaseModel, Generic[FlagT]):
    flags: List[FlagT] = Field(default_factory=list)

    def model_post_init(self, __context: Any) -> None:
        seen_flags = set()
        for flag in self.flags:
            if flag.name in seen_flags:
                raise ValueError(
                    f"Duplicate flag! {flag.name=} was defined more than once!"
                )

        return super().model_post_init(__context)


class ClientConfig(Config[AnnotatedClientFlag]):
    data: ClientData

    def get_flag_combinations(self) -> List[str]:
        raise NotImplementedError


class HostConfig(Config[AnnotatedHostFlag]):
    pass


class ExperimentMetatadata(BaseModel):
    description: str = ""


class Experiment(BaseModel):
    name: str
    output_directory: Path
    metadata: ExperimentMetatadata

    client_config: ClientConfig
    host_config: HostConfig

    def get_client_flag_combinations(self) -> List[str]:
        return ClientConfig.get_flag_combinations()


def load_experiments(experiment_paths: List[ExperimentPath]) -> List[Experiment]:
    experiments: List[Experiment] = []
    for e in experiment_paths:
        with open(e.experiment_path, "r") as f:
            loaded_experiment = yaml.safe_load(f)

        experiment = Experiment.model_validate(loaded_experiment)
        experiments.append(experiment)

    return experiments
