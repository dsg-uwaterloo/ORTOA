from abc import ABC, abstractmethod
from typing import Generic, List, Literal, TypeVar, Union

from pydantic import BaseModel, Field

T = TypeVar("T", bound=Union[int, str, bool, float])


##########################
# Abstractions
##########################


class IntType(BaseModel):
    type: Literal["int"] = Field(default="int", frozen=True)


class Parameter(BaseModel, ABC):
    @abstractmethod
    def generate_values(self) -> List:
        raise NotImplementedError


NumberT = TypeVar("NumberT", bound=Union[int, float])


class RangeParameter(Parameter, Generic[NumberT]):
    minimum: NumberT
    maximum: NumberT


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
