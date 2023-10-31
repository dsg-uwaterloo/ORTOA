import yaml
from pathlib import Path
from typing import Any

from benchmark.interface.data import ByteSizeGenerationConfig
from benchmark.interface.parameter import (
    IntegerParameter,
    IntegerIncrementRange,
    IntegerMultiplyRange,
)


def test_data_generation_config():
    def get_path(filename: str) -> Path:
        return Path(f"benchmark/test/interface/test_files/{filename}.yaml")

    def load_yaml(file_path: Path) -> Any:
        with open(file_path, "r") as f:
            loaded_yaml = yaml.safe_load(f)
        return loaded_yaml

    # assert can deserialize ByteSizeGenerator
    bsg_static_int_yaml = load_yaml(get_path("byte_size_datagen_int"))
    config = ByteSizeGenerationConfig.model_validate(bsg_static_int_yaml)
    assert config.generator == "ByteSizeGenerator"
    assert isinstance(config.generator, IntegerParameter)
    assert isinstance(config.generator.value, int)
    assert config.generator.value == 160

    bsg_int_increment_range_yaml = load_yaml(get_path("byte_size_datagen_int_range"))
    config = ByteSizeGenerationConfig.model_validate(bsg_int_increment_range_yaml)
    assert config.generator == "ByteSizeGenerator"
    assert isinstance(config.generator, IntegerIncrementRange)
    assert config.n_bytes.type == "int"
    assert config.n_bytes.minimum == 100
    assert config.n_bytes.maximum == 500
    assert config.n_bytes.step == 20

    bsg_int_multiply_range_yaml = load_yaml(get_path("byte_size_datagen_int_multiple"))
    config = ByteSizeGenerationConfig.model_validate(bsg_int_multiply_range_yaml)
    assert config.generator == "ByteSizeGenerator"
    assert isinstance(config.generator, IntegerMultiplyRange)
    assert config.n_bytes.type == "int"
    assert config.n_bytes.minimum == 100
    assert config.n_bytes.maximum == 500
    assert config.n_bytes.multiplier == 2

    # assert can deserialize RandomIntegerGenerator


def test_flag_deserialization():
    # Test every flag individually
    # Test a list containing every type of flag (ensure that pydantic correctly discriminates in the union)
    pass


def test_client_config():
    # Test that a ClientConfig can be deserialized
    pass


def test_host_config():
    # Test that a HostConfig can be deserialized
    pass


def test_experiment_deserialization():
    # Test that an Experiment can be deserialized
    pass
