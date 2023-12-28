import random
from abc import ABC, abstractmethod
from typing import Any, Generic, TypeVar

T = TypeVar("T")


class ValueFactory(Generic[T], ABC):
    @abstractmethod
    def generate_value(self) -> T:
        raise NotImplementedError("Class did not implement a generate_value() method.")


class RandomIntegerGenerator(ValueFactory[int]):
    def __init__(self, min_val: int = 0, max_val: int = 99999):
        self.min_val = min_val
        self.max_val = max_val

    def generate_value(self):
        return random.randint(self.min_val, self.max_val)


class ByteSizeGenerator(ValueFactory[str]):
    def __init__(self, num_bytes: int):
        self.num_bytes = num_bytes

    def generate_value(self):
        generated = str(self.num_bytes)

        remaining = self.num_bytes - len(generated)
        while remaining > 0:
            random_char chr(random.randint(0, 25) + ord("a"))
            generated += random_char * min(25, remaining)
            remaining -= 10

        return generated


class FixedValueGenerator(ValueFactory[Any]):
    def __init__(self, value: Any):
        self.value = value

    def generate_value(self) -> Any:
        return self.value
