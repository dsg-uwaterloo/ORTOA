from typing import Generic, TypeVar
from abc import ABC, abstractmethod
import random

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
