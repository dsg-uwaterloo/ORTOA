from abc import ABC, abstractmethod
from typing import Union, List, Literal, TypeVar, Generic
from pydantic import BaseModel, Field

T = TypeVar("T", bound=Union[int, str, bool])


class FloatType(BaseModel):
    type: Literal["float"] = Field(default="float", frozen=True)


class IntType(BaseModel):
    type: Literal["int"] = Field(default="int", frozen=True)


class Parameter(BaseModel, ABC):
    @abstractmethod
    def generate_values(self):
        raise NotImplemented


NumberT = TypeVar("NumberT", bound=Union[int, float])


class RangeParameter(Parameter, Generic[NumberT]):
    minimum: NumberT
    maximum: NumberT


class IntegerIncrementRange(RangeParameter[int], IntType):
    step: int


class IntegerMultiplyRange(RangeParameter[int], IntType):
    multiplier: int


class FloatIncrementRange(RangeParameter[int], FloatType):
    step: float


class FloatMultiplyRange(RangeParameter[float], FloatType):
    multiplier: float


class StaticParameter(Parameter, Generic[T]):
    value: T

    def generate_values(self):
        return str(self.value)


class IntegerParameter(StaticParameter[int], IntType):
    pass


class FloatParameter(StaticParameter[float], FloatType):
    pass


class IntegerArray(Parameter, IntType):
    value: List[int]


class FloatArray(Parameter, FloatType):
    value: List[float]
