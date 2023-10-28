from pydantic import BaseModel

from benchmark.interface.experiment import Experiment


class Job(BaseModel):
    experiment: Experiment
