from abc import ABC, abstractmethod
from typing import Literal, Union

from pydantic import Annotated, BaseModel, Field


class Flag(BaseModel, ABC):
    @abstractmethod
    def __str__(self):
        raise NotImplementedError("Cannot get string repr of abstract Flag class")


#########################
# Client Flags
#########################


class ClientFlag(Flag):
    pass


class NClientThreads(ClientFlag):
    name: Literal["nthreads"] = Field(default="nthreads", frozen=True)
    # TODO: Value

    def __str__(self):
        raise NotImplementedError("Haven't implemented NClientThreads flag yet!")


class PGet(ClientFlag):
    name: Literal["pget"] = Field(default="pget", frozen=True)
    # TODO: Value

    def __str__(self):
        raise NotImplementedError("Haven't implemented PGet flag yet!")


class ClientLoggingEnabled(ClientFlag):
    # TODO: Name
    # TODO: Value

    def __str__(self):
        raise NotImplementedError("Haven't implemented ClientLoggingEnabled flag yet!")


AnnotatedClientFlag = Annotated[
    Union[NClientThreads, PGet, ClientLoggingEnabled],
    Field(discriminator="name"),
]

#########################
# Host Flags
#########################


class HostFlag(Flag):
    pass


class NHostThreads(HostFlag):
    name: Literal["nthreads"] = Field(default="nthreads", frozen=True)
    # TODO: VAlue

    def __str__(self):
        raise NotImplementedError("Haven't implemented NHostThreads flag yet!")


class HostLoggingEnabled(HostFlag):
    # TODO: Name
    # TODO: Value

    def __str__(self):
        raise NotImplementedError("Haven't implemented HostLoggingEnabled flag yet!")


AnnotatedHostFlag = Annotated[
    Union[NHostThreads, HostLoggingEnabled], Field(discriminator="name")
]
