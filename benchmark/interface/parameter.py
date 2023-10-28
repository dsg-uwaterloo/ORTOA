from abc import ABC, abstractmethod
from typing import Union, List
from pydantic import BaseModel


class Parameter(BaseModel, ABC):
    @abstractmethod
    def generate_value():
        pass


class IntRange(Parameter):
    minimum: int
    maximum: int
    step: int


class FloatRange(Parameter):
    minimum: float
    maximum: float
    step: float


class StaticInteger(Parameter):
    value: int


class StaticFloat(Parameter):
    value: float


class ArrayOfInts(Parameter):
    value: List[int]
