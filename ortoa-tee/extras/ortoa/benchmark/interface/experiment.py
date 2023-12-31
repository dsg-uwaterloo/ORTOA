from pathlib import Path
from typing import Any, Generic, List, Literal, Optional, TypeVar, Union

import yaml
from pydantic import BaseModel, Field
from typing_extensions import Self

from ortoa.benchmark.infrastucture.experiment_collection import ExperimentPath
from ortoa.benchmark.interface.data import DataGenConfig, DataGenerationConfigBase
from ortoa.benchmark.interface.flags import AnnotatedClientFlag

FlagT = TypeVar("FlagT", bound=AnnotatedClientFlag)


class SeedData(BaseModel):
    data_type: Literal["seed"] = Field(default="seed", frozen=True, init_var=False)
    seed: Optional[Path]
    operations: Optional[Path]

    @classmethod
    def from_generation_config(
        cls, data: DataGenerationConfigBase, output_dir: Path
    ) -> Self:
        seed, operations = data.generate_files(output_dir)
        return cls(seed=seed, operations=operations)


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
    data: Union[SeedData, DataGenConfig]

    def get_flag_combinations(self) -> List[str]:
        raise NotImplementedError


class ExperimentMetatadata(BaseModel):
    description: str = ""
    nbytes: Optional[int] = None
    db_size: Optional[str] = None
    percent_write: Optional[str] = None


class Experiment(BaseModel):
    name: str
    output_directory: Path
    metadata: ExperimentMetatadata

    client_config: ClientConfig

    def get_client_flag_combinations(self) -> List[str]:
        return self.client_config.get_flag_combinations()

    def generate_data(self) -> None:
        if isinstance(self.client_config.data, DataGenerationConfigBase):
            output_file: Path = self.output_directory / "generated_data"
            output_file.mkdir(parents=True, exist_ok=True)

            self.client_config.data = SeedData.from_generation_config(
                self.client_config.data,
                output_file,
            )

            if (
                self.client_config.data.seed
                and not self.client_config.data.seed.exists()
            ):
                raise FileNotFoundError(
                    f"Seed file {self.client_config.data.seed} was not found."
                )

            if (
                self.client_config.data.operations
                and not self.client_config.data.operations.exists()
            ):
                raise FileNotFoundError(
                    f"Operations file {self.client_config.data.operations} was not found."
                )


def load_experiments(
    experiment_paths: List[ExperimentPath], base_dir: Path
) -> List[Experiment]:
    experiments: List[Experiment] = []
    for e in experiment_paths:
        with open(e.experiment_path, "r") as f:
            loaded_experiment = yaml.safe_load(f)

        experiment = Experiment.model_validate(loaded_experiment)
        experiment.output_directory = base_dir / experiment.output_directory
        experiments.append(experiment)

    return experiments


class AtomicExperiment(BaseModel):
    name: str
    output_directory: Path
    metadata: ExperimentMetatadata

    seed_data: Path
    operations: Path

    client_flags: List[AnnotatedClientFlag]


def combine(lst):
    combinations = []

    def backtrack(curr, idx: int):
        nonlocal lst, combinations

        if idx == len(lst):
            combinations.append(curr[:])
            return

        for i in range(len(lst[idx])):
            curr.append(lst[idx][i])
            backtrack(curr, idx + 1)
            curr.pop()

    tmp = []
    backtrack(tmp, 0)
    return combinations


def atomicize_experiments(experiments: List[Experiment]) -> List[AtomicExperiment]:
    atomic_experiments: List[AtomicExperiment] = []
    for experiment in experiments:
        assert isinstance(experiment.client_config.data, SeedData)
        assert experiment.client_config.data.seed is not None
        assert experiment.client_config.data.operations is not None

        all_client_flags = [
            flag.get_atomic_flags() for flag in experiment.client_config.flags
        ]

        client_flag_combinations = combine(all_client_flags)

        _id = 0
        for cflags in client_flag_combinations:
            atomic_experiments.append(
                AtomicExperiment(
                    name=experiment.name,
                    output_directory=experiment.output_directory / f"combination_{_id}",
                    metadata=experiment.metadata,
                    seed_data=experiment.client_config.data.seed,
                    operations=experiment.client_config.data.operations,
                    client_flags=cflags,
                )
            )
            _id += 1

    return atomic_experiments
