# Data Directory

## Top-level directory layout

```txt
.
├── experiments                # Benchmarking experiments
├── sample                     # Sample end-to-end benchmarking flow
└── README.md
```

## `experiments`

We define experiments in `.yaml` format used to microbenchmark the ORTOA-TEE program. These are consumed by the benchmarking tool to test how certain variables affect the latency and throughput of ORTOA.

```txt
.
├── ...
├── experiments
│   ├── byte_size              # Varying the size of the values (in bytes)
│   ├── client_concurrency     # Varying the number of concurrent clients
│   ├── db_size                # Varying the number of keys in the DB
│   ├── percent_write          # Varying the % of read vs. write operations
│   ├── real_world_data        # Benchmarking ORTOA-TEE on real-world data
│   └── server_location        # Benchmarking ORTOA-TEE as network latency increases
└── ...
```

## `sample`

A sample end-to-end benchmarking flow complete with a seed file, an operations file, a benchmarking `.yaml` config and a `README` with the relevant commands.
