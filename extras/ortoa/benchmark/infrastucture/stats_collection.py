from functools import reduce
from pathlib import Path
from typing import List

import numpy as np
import pandas as pd
from pydantic import BaseModel
from typing_extensions import Self

from ortoa.benchmark.infrastucture.jobs import ClientJob
from ortoa.benchmark.infrastucture.runner import Result


class Stats(BaseModel):
    class Config:
        arbitrary_types_allowed = True

    raw_df: pd.DataFrame  # Entry from every experiment

    def _graph_threads_vs_latency(self, dir: Path) -> None:
        df = self.raw_df.sort_values(by=['nthreads'], ascending=True)
        ax = df.plot.bar(x="nthreads", y="average_latency")
        ax.set_ylabel("Latency (ms)")
        fig = ax.get_figure()
        fig.savefig(dir / "threads_vs_latency.pdf")

    def _graph_threading_effects(self, dir: Path) -> None:
        df = self.raw_df.sort_values(by=['nthreads'])
        ax = df.plot(
            x="nthreads",
            y=["average_latency", "throughput"],
            secondary_y=["average_latency"],
            kind="bar",
        )
        ax.set_ylabel("Throughput (ops/s)")
        ax.right_ax.set_ylabel("Latency (ms)")
        fig = ax.get_figure()
        fig.savefig(dir / "threading_effects.pdf")

    def _graph_byte_size(self, dir: Path) -> None:
        df = self.raw_df.sort_values(by=['bytes'], ascending=True)
        ax = df.plot.bar(
            x="bytes",
            y=["average_latency", "throughput"],
            secondary_y=["average_latency"],
        )
        ax.set_ylabel("Throughput (ops/s)")
        ax.right_ax.set_ylabel("Latency (ms)")
        fig = ax.get_figure()
        fig.savefig(dir / "byte_size.pdf")
    
    def _graph_db_size(self, dir: Path) -> None:
        df = self.raw_df.sort_values(by=['db_size'], ascending=True)
        ax = df.plot.bar(
            x="db_size",
            y=["average_latency", "throughput"],
            secondary_y=["average_latency"],
        )
        ax.set_ylabel("Throughput (ops/s)")
        ax.right_ax.set_ylabel("Latency (ms)")
        fig = ax.get_figure()
        fig.savefig(dir / "db_size.pdf")
    
    def _graph_percent_write(self, dir: Path) -> None:
        df = self.raw_df
        df["percent_write"] = df["percent_write"].apply(lambda x: int(x[:-1]))
        df = df.sort_values(by=["percent_write"], ascending=True)
        ax = df.plot.bar(
            x="percent_write",
            y=["average_latency", "throughput"],
            secondary_y=["average_latency"],
        )
        ax.set_ylabel("Throughput (ops/s)")
        ax.right_ax.set_ylabel("Latency (ms)")
        fig = ax.get_figure()
        fig.savefig(dir / "percent_write.pdf")

    def _save_graphs(self, dir: Path) -> None:
        self._graph_threads_vs_latency(dir)
        self._graph_threading_effects(dir)

        if self.raw_df["bytes"][0] is not None:
            self._graph_byte_size(dir)
        
        if self.raw_df["db_size"][0] is not None:
            self._graph_db_size(dir)
        
        if self.raw_df["percent_write"][0] is not None:
            self._graph_percent_write(dir)

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

        throughput = 1000 * num_operations / total_time  # measured in ops/second

        result_summary = pd.DataFrame(
            {
                "seed": [job.client_flags.seed],
                "operations": [job.client_flags.operations],
                "seed_size": [seed_size],
                "db_size": [job.metadata.db_size],
                "percent_write": [job.metadata.percent_write],
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
