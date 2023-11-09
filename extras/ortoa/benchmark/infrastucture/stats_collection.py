from pydantic import BaseModel, Field
from typing import List
from typing_extensions import Self
from pathlib import Path
import pandas as pd
import numpy as np
from functools import reduce

from ortoa.benchmark.infrastucture.runner import Result
from ortoa.benchmark.infrastucture.jobs import ClientJob

from icecream import ic


class Stats(BaseModel):
    class Config:
        arbitrary_types_allowed = True

    raw_df: pd.DataFrame  # Entry from every experiment
    # summary_df: pd.DataFrame = Field(init_var=False, required=False)

    def save_to(self, dir: Path) -> None:
        self.raw_df.to_csv(dir / "complete.csv")
        # self.summary_df.to_csv(dir / "summary.csv")

    @classmethod
    def _parse_result(self, job: ClientJob, results_file: Path) -> pd.DataFrame:
        """Parse the results from C++ and add them to the dataframe"""

        with results_file.open("r") as f:
            latencies_line = f.readline()
            all_latencies = sorted([int(l) for l in latencies_line.split(",")[:-1]])
            average_latency = float(f.readline().strip())

        latency_stdev = np.std(all_latencies)

        result_summary = pd.DataFrame(
            {
                "seed": [job.client_flags.seed],
                "operations": [job.client_flags.operations],
                "nthreads": [job.client_flags.nthreads],
                "average_latency": [average_latency],
                "latency std": [latency_stdev],
            }
        )

        print(result_summary)

        return result_summary

    @classmethod
    def from_results(cls, results: List[Result]) -> Self:
        """Given an iterable of results, generate statistics on those results"""
        per_job_result = [
            cls._parse_result(job=result.job, results_file=result.result_path)
            for result in results
        ]
        aggregated_results = reduce(
            lambda x, y: pd.merge(x, y, how="outer"), per_job_result
        )
        return cls(raw_df=aggregated_results)
