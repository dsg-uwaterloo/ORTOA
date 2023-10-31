from typing import Union, Literal, Tuple
from typing_extensions import Annotated
from pydantic import BaseModel, Field
from abc import ABC, abstractmethod
from benchmark.interface.parameter import (
    IntegerIncrementRange,
    IntegerParameter,
    IntegerMultiplyRange,
)
from pathlib import Path


class DataGenerationConfigBase(BaseModel, ABC):
    seed_size: int
    num_operations: int
    key_access_distribution: Literal["uniform"]

    @abstractmethod
    def generate_files(self, output_dir: Path) -> Tuple[Path, Path]:
        raise NotImplementedError


class ByteSizeGenerationConfig(DataGenerationConfigBase):
    generator: Literal["ByteSizeGenerator"]
    n_bytes: IntegerParameter

    def generate_files(self, output_dir: Path) -> Tuple[Path, Path]:
        raise NotImplementedError


class RandomIntegerGenerationConfig(DataGenerationConfigBase):
    generator: Literal["RandomIntegerGenerator"]
    minimum: int = 0
    maximum: int = 100000

    def generate_files(self, output_dir: Path) -> Tuple[Path, Path]:
        raise NotImplementedError


DataGenConfig = Annotated[
    Union[ByteSizeGenerationConfig, RandomIntegerGenerationConfig],
    Field(discriminator="generator"),
]
