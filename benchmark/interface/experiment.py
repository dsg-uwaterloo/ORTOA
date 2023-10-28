from abc import ABC, abstractmethod
from typing import Union, List, Optional
from pydantic import BaseModel, Field
from pathlib import Path

from benchmark.interface.flags import AnnotatedClientFlag, AnnotatedHostFlag


class ClientData(BaseModel):
    seed: Optional[Path] = None
    operations: Optional[Path] = None


class ClientConfig(BaseModel):
    data: ClientData
    flags: List[AnnotatedClientFlag] = Field(default_factory=list)


class HostConfig(BaseModel):
    flags: List[AnnotatedHostFlag] = Field(default_factory=list)
    pass


class Experiment(BaseModel):
    client_config: ClientConfig
    host_config: HostConfig
