from abc import ABC, abstractmethod
from typing import List, Literal, Union

from pydantic import BaseModel, Field
from typing_extensions import Annotated, Self

from ortoa.benchmark.interface.parameter import (
    IntegerIncrementRange,
    IntegerMultiplyRange,
)


class Flag(BaseModel, ABC):
    @abstractmethod
    def __str__(self):
        raise NotImplementedError("Cannot get string repr of abstract Flag class")

    @abstractmethod
    def get_atomic_flags(self):
        raise NotImplementedError("Cannot get atomic version of abstract Flag class")


#########################
# Client Flags
#########################


class ClientFlag(Flag):
    pass


class NClientThreads(ClientFlag):
    name: Literal["nthreads"] = Field(default="nthreads", frozen=True)
    value: Union[int, IntegerIncrementRange, IntegerMultiplyRange]

    def __str__(self):
        if not isinstance(self.value, int):
            raise ValueError("The value of the Client flag --nthreads is not atomic")

        return f"--nthreads {self.value}"

    def get_atomic_flags(self) -> List[Self]:
        atomic_selfs: List[Self] = []
        if isinstance(self.value, int):
            atomic_selfs.append(NClientThreads(name=self.name, value=self.value))
        elif isinstance(self.value, (IntegerIncrementRange, IntegerMultiplyRange)):
            for val in self.value.generate_values():
                atomic_selfs.append(NClientThreads(name=self.name, value=val))
        else:
            raise TypeError(
                "NClientThreads::get_atomic_flags() did not recognize type of self.value"
            )

        return atomic_selfs


class ClientLoggingEnabled(ClientFlag):
    name: Literal["client_logging_enabled"] = Field(
        default="client_logging_enabled", frozen=True
    )
    value: bool

    def __str__(self):
        return f"--logging_enabled {str(self.value)}"

    def get_atomic_flags(self) -> List[Self]:
        return [self]


AnnotatedClientFlag = Annotated[
    Union[NClientThreads, ClientLoggingEnabled],
    Field(discriminator="name"),
]
