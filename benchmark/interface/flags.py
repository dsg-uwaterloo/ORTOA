from abc import ABC, abstractmethod
from pydantic import BaseModel, Field, Annotated
from typing import Literal, Union


class Flag(BaseModel, ABC):
    @abstractmethod
    def __str__(self):
        raise NotImplementedError("Cannot get string repr of abstract Flag class")


#########################
# Client Flags
#########################


class ClientFlag(Flag):
    pass


class InitDB(ClientFlag):
    name: Literal["initdb"] = Field(default="initdb", frozen=True)
    value: bool = False

    def __str__(self):
        raise NotImplementedError("Haven't implemented InitDB flag yet!")


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
    Union[InitDB, NClientThreads, PGet, ClientLoggingEnabled],
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
