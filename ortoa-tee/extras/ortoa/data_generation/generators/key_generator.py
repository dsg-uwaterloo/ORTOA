from abc import ABC, abstractmethod
from typing import Generic, TypeVar

T = TypeVar("T")


class KeyFactory(Generic[T], ABC):
    @abstractmethod
    def generate_key(self) -> T:
        raise NotImplementedError("Class did not implement a generate_key() method")


class SequentialIntKeyGenerator(KeyFactory[int]):
    def __init__(self, start_key: int = 1):
        self.key = start_key

    def generate_key(self) -> int:
        generated_key = self.key
        self.key += 1
        return generated_key
