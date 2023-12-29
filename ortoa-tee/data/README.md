# Data Directory

## Top-level directory layout

```txt
.
├── experiments                # Benchmarking experiments
├── sample_seed                # Sample seed and operations files
├── tests                      # Tests used while developing ORTOA
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
│   └── percent_write          # Varying the % of read vs. write operations
└── ...
```

## `seed`

Sample seed and operations `.csv` files that are consumed by the `ortoa-client` program to perform database accesses with the ORTOA protocol.

## `tests`

Tests defined in the same `.yaml` format as `experiments`. Consumed by the benchmarking tool while developing ORTOA-TEE to test the success/failure of the program.

```txt
.
├── ...
├── tests
│   └── byte_size_tests        # How big can we push the size of the values?
└── ...
```
