from pydantic import BaseModel, Field
from typing import List
from typing_extensions import Self
from ortoa.benchmark.infrastucture.runner import Result
from pathlib import Path
import pandas as pd


class Stats(BaseModel):
    class Config:
        arbitrary_types_allowed = True

    raw_df: pd.DataFrame  # Entry from every experiment
    summary_df: pd.DataFrame = Field(init_var=False)

    def save_to(self, dir: Path) -> None:
        self.raw_df.to_csv(dir / "complete.csv")
        self.summary_df.to_csv(dir / "summary.csv")

    @classmethod
    def from_results(cls, results: List[Result]) -> Self:
        """Given an iterable of results, generate statistics on those results"""
        raise NotImplementedError
