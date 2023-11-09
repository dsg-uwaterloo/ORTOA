from typing import Any, Union, Literal, Tuple, Optional
from typing_extensions import Annotated
from pydantic import BaseModel, Field
from abc import ABC, abstractmethod
from ortoa.benchmark.interface.parameter import (
    IntegerIncrementRange,
    IntegerParameter,
    IntegerMultiplyRange,
)
from pathlib import Path

from ortoa.data_generation.generate_seed_data import generate_data
from ortoa.data_generation.generate_sample_operations import generate_operations
from ortoa.data_generation.generators.key_generator import SequentialIntKeyGenerator
from ortoa.data_generation.generators.value_generator import (
    RandomIntegerGenerator,
    ByteSizeGenerator,
)


class DataGenerationConfigBase(BaseModel, ABC):
    seed_size: int
    num_operations: int
    key_access_distribution: Literal["uniform"] = Field(default="uniform")

    @abstractmethod
    def generate_files(self, output_dir: Path) -> Tuple[Path, Path]:
        """Generates the seed and operations files based on the configuration object

        Returns:
            Tuple[Path, Path]: tuple containing (seed_data, operations_data)
        """
        raise NotImplementedError


class ByteSizeGenerationConfig(DataGenerationConfigBase):
    generator: Literal["ByteSizeGenerator"]
    n_bytes: int

    def _generate_seed(self, output_file: Path) -> Path:
        key_generator = SequentialIntKeyGenerator()
        value_generator = ByteSizeGenerator(self.n_bytes)
        generate_data(
            key_generator=key_generator,
            value_generator=value_generator,
            num_data_points=self.seed_size,
            output_file=output_file,
        )

        return output_file

    def _generate_operations(self, seed_file: Path, output_file: Path) -> Path:
        value_generator = ByteSizeGenerator(self.n_bytes)
        generate_operations(
            num_operations=self.num_operations,
            input_file=seed_file,
            output_file=output_file,
            p_get=0.5,
            value_generator=value_generator,
        )

        return output_file

    def generate_files(self, output_dir: Path) -> Tuple[Path, Path]:
        seed = self._generate_seed(output_dir / "seed.csv")
        operations = self._generate_operations(seed, output_dir / "operations.csv")

        return seed, operations


class RandomIntegerGenerationConfig(DataGenerationConfigBase):
    generator: Literal["RandomIntegerGenerator"]
    minimum: int
    maximum: int

    def _generate_seed(self, output_file: Path) -> Path:
        key_generator = SequentialIntKeyGenerator()
        value_generator = RandomIntegerGenerator(
            min_val=self.minimum, max_val=self.maximum
        )
        generate_data(
            key_generator=key_generator,
            value_generator=value_generator,
            num_data_points=self.seed_size,
            output_file=output_file,
        )

        return output_file

    def _generate_operations(self, seed_file: Path, output_file: Path) -> Path:
        value_generator = RandomIntegerGenerator(
            min_val=self.minimum, max_val=self.maximum
        )
        generate_operations(
            num_operations=self.num_operations,
            input_file=seed_file,
            output_file=output_file,
            p_get=0.5,
            value_generator=value_generator,
        )

        return output_file

    def generate_files(self, output_dir: Path) -> Tuple[Path, Path]:
        seed = self._generate_seed(output_dir / "seed.csv")
        operations = self._generate_operations(seed, output_dir / "operations.csv")

        return seed, operations


DataGenConfig = Annotated[
    Union[ByteSizeGenerationConfig, RandomIntegerGenerationConfig],
    Field(discriminator="generator"),
]
