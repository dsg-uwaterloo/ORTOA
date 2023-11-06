from pydantic import BaseModel, Field
from typing import List
from typing_extensions import Self
from ortoa.benchmark.infrastucture.runner import Result

import pandas as pd


class Stats(BaseModel):
    raw_df: pd.DataFrame  # Entry from every experiment
    summary_df: pd.DataFrame = Field(init_var=False)

    @classmethod
    def from_results(cls, results: List[Result]) -> Self:
        """Given an iterable of results, generate statistics on those results"""
        raise NotImplementedError
