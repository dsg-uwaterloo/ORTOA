import yaml
from pathlib import Path
from typing import Any, List, Union
from typing_extensions import Annotated
import pytest

from benchmark.interface.data import (
    ByteSizeGenerationConfig,
    RandomIntegerGenerationConfig,
)
from benchmark.interface.parameter import (
    IntegerParameter,
    IntegerIncrementRange,
    IntegerMultiplyRange,
)

from pydantic import BaseModel, Field


def get_path(filename: str) -> Path:
    return Path(f"benchmark/test/interface/test_files/{filename}.yaml")


def load_yaml(file_path: Path) -> Any:
    with open(file_path, "r") as f:
        loaded_yaml = yaml.safe_load(f)
    return loaded_yaml


def test_basic_data_generation_config():
    # assert can deserialize ByteSizeGenerator
    bsg_static_int_yaml = load_yaml(get_path("byte_size_datagen_int"))
    config = ByteSizeGenerationConfig.model_validate(bsg_static_int_yaml)
    assert config.generator == "ByteSizeGenerator"
    assert isinstance(config.n_bytes, IntegerParameter)
    assert isinstance(config.n_bytes.value, int)
    assert config.n_bytes.value == 160

    # Check can deserialize RandomIntegerGenerator
    rig_static_int_yaml = load_yaml(get_path("random_integer_datagen"))
    config = RandomIntegerGenerationConfig.model_validate(rig_static_int_yaml)
    assert config.generator == "RandomIntegerGenerator"
    assert config.minimum == 10
    assert config.maximum == 100
    assert config.seed_size == 100
    assert config.num_operations == 20
    assert config.key_access_distribution == "uniform"


def test_datagen_config_discrimination():
    class MockClientConfig(BaseModel):
        data_generations: List[
            Annotated[
                Union[RandomIntegerGenerationConfig, ByteSizeGenerationConfig],
                Field(discriminator="generator"),
            ]
        ]

    bsg_static_int_yaml = load_yaml(get_path("byte_size_datagen_int"))
    rig_static_int_yaml = load_yaml(get_path("random_integer_datagen"))
    mock_list = {"data_generations": [bsg_static_int_yaml, rig_static_int_yaml]}

    model = MockClientConfig.model_validate(mock_list)
    assert isinstance(model.data_generations[0], ByteSizeGenerationConfig)
    assert isinstance(model.data_generations[1], RandomIntegerGenerationConfig)


@pytest.mark.xfail
def test_range_data_generation_config():
    bsg_int_increment_range_yaml = load_yaml(get_path("byte_size_datagen_int_range"))
    config = ByteSizeGenerationConfig.model_validate(bsg_int_increment_range_yaml)
    assert config.generator == "ByteSizeGenerator"
    assert isinstance(config.n_bytes, IntegerIncrementRange)
    assert config.n_bytes.type == "int"
    assert config.n_bytes.minimum == 100
    assert config.n_bytes.maximum == 500
    assert config.n_bytes.step == 20

    bsg_int_multiply_range_yaml = load_yaml(get_path("byte_size_datagen_int_multiple"))
    config = ByteSizeGenerationConfig.model_validate(bsg_int_multiply_range_yaml)
    assert config.generator == "ByteSizeGenerator"
    assert isinstance(config.n_bytes, IntegerMultiplyRange)
    assert config.n_bytes.type == "int"
    assert config.n_bytes.minimum == 100
    assert config.n_bytes.maximum == 500
    assert config.n_bytes.multiplier == 2


# def test_flag_deserialization():
#     # Test every flag individually
#     # Test a list containing every type of flag (ensure that pydantic correctly discriminates in the union)
#     pass


# def test_client_config():
#     # Test that a ClientConfig can be deserialized
#     pass


# def test_host_config():
#     # Test that a HostConfig can be deserialized
#     pass


# def test_experiment_deserialization():
#     # Test that an Experiment can be deserialized
#     pass
