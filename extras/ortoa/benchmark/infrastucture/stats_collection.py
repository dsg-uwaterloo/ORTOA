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

import random


class Stats(BaseModel):
    class Config:
        arbitrary_types_allowed = True

    raw_df: pd.DataFrame  # Entry from every experiment
    # summary_df: pd.DataFrame = Field(init_var=False, required=False)

    def _graph_threads_vs_latency(self, dir: Path) -> None:
        ax = self.raw_df.plot.bar(x="nthreads", y="average_latency")
        fig = ax.get_figure()
        fig.savefig(dir / "threads_vs_latency.pdf")

    def _graph_threading_effects(self, dir: Path) -> None:
        ax = self.raw_df.plot(
            x="nthreads",
            y=["average_latency", "throughput"],
            secondary_y=["throughput"],
            kind="bar",
        )
        fig = ax.get_figure()
        fig.savefig(dir / "threading_effects.pdf")

    def _graph_byte_size(self, dir: Path) -> None:
        ax = self.raw_df.plot.bar(
            x="bytes",
            y=["average_latency", "throughput"],
            secondary_y=["average_latency"],
        )
        fig = ax.get_figure()
        fig.savefig(dir / "byte_size.pdf")

    def _save_graphs(self, dir: Path) -> None:
        self._graph_threads_vs_latency(dir)
        self._graph_threading_effects(dir)

        if self.raw_df["bytes"][0] is not None:
            self._graph_byte_size(dir)

    def save_to(self, dir: Path) -> None:
        self.raw_df.to_csv(dir / "complete.csv")
        self._save_graphs(dir=dir)

    @classmethod
    def _parse_result(self, job: ClientJob, results_file: Path) -> pd.DataFrame:
        """Parse the results from C++ and add them to the dataframe"""

        with results_file.open("r") as f:
            # All latencies
            latencies_line = f.readline()
            all_latencies = sorted([int(l) for l in latencies_line.split(",")[:-1]])

            # Average latency
            average_latency = float(f.readline().strip())

            # Total time
            total_time = int(f.readline())

        latency_stdev = np.std(all_latencies)

        with job.seed_data.open("r") as f:
            seed_size = len(f.readlines())

        with job.operations.open("r") as f:
            num_operations = len(f.readlines())

        throughput = 1000000 * num_operations / total_time  # measured in ops/second

        result_summary = pd.DataFrame(
            {
                "seed": [job.client_flags.seed],
                "operations": [job.client_flags.operations],
                "seed_size": [seed_size],
                "num_operations": [num_operations],
                "bytes": [job.metadata.nbytes],
                "nthreads": [job.client_flags.nthreads],
                "average_latency": [average_latency],
                "latency std": [latency_stdev],
                "throughput": [throughput],
            }
        )

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
