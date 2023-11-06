from abc import ABC, abstractmethod
from typing import Generic, List, Literal, TypeVar, Union

from pydantic import BaseModel, Field

T = TypeVar("T", bound=Union[int, str, bool])


##########################
# Abstractions
##########################


class FloatType(BaseModel):
    type: Literal["float"] = Field(default="float", frozen=True)


class IntType(BaseModel):
    type: Literal["int"] = Field(default="int", frozen=True)


class Parameter(BaseModel, ABC):
    @abstractmethod
    def generate_values(self) -> List[str]:
        raise NotImplementedError


NumberT = TypeVar("NumberT", bound=Union[int, float])


class RangeParameter(Parameter, Generic[NumberT]):
    minimum: NumberT
    maximum: NumberT


class StaticParameter(Parameter, Generic[T]):
    value: T

    def generate_values(self):
        return [str(self.value)]


# class ArrayParameter(Parameter, Generic[T]):
#     value: List[T]

#     def generate_values(self) -> List[str]:
#         return [str(v) for v in self.value]

##########################
# Parameter Types
##########################


class IntegerIncrementRange(RangeParameter[int], IntType):
    step: int

    def generate_values(self) -> List[int]:
        res: List[int] = []

        i = self.minimum
        while i <= self.maximum:
            res.append(i)
            i += self.step

        return res


class IntegerMultiplyRange(RangeParameter[int], IntType):
    multiplier: int

    def generate_values(self) -> List[int]:
        res: List[int] = []

        i = self.minimum
        while i <= self.maximum:
            res.append(i)
            i *= self.multiplier

        return res


class FloatIncrementRange(RangeParameter[int], FloatType):
    step: float

    def generate_values(self) -> List[float]:
        res: List[float] = []

        i = self.minimum
        while i <= self.maximum:
            res.append(i)
            i += self.step

        return res


class FloatMultiplyRange(RangeParameter[float], FloatType):
    multiplier: float

    def generate_values(self) -> List[str]:
        res: List[float] = []

        i = self.minimum
        while i <= self.maximum:
            res.append(i)
            i *= self.multiplier

        return res


class IntegerParameter(StaticParameter[int], IntType):
    pass


class FloatParameter(StaticParameter[float], FloatType):
    pass


# class IntegerArray(ArrayParameter[int], IntType):
#     pass


# class FloatArray(ArrayParameter[float], FloatType):
#     pass
