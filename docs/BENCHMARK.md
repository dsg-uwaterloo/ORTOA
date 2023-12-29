# ORTOA-TEE Benchmarking

```txt
usage: main.py [-h] [-e EXPERIMENTS [EXPERIMENTS ...]] [-d EXPERIMENT_DIRS [EXPERIMENT_DIRS ...]] [-w WORKING_DIR]

optional arguments:
  -h, --help            show this help message and exit
  -w WORKING_DIR, --working-dir WORKING_DIR
                        Directory to use as base for experiment directory tree (default: out/benchmark-2023-12-29-11-38-05)

Experiments:
  Options to control experiments selected for compilation

  -e EXPERIMENTS [EXPERIMENTS ...], --experiments EXPERIMENTS [EXPERIMENTS ...]
                        List of experiments to compile (experiment name should match zoo object)
  -d EXPERIMENT_DIRS [EXPERIMENT_DIRS ...], --experiment-dirs EXPERIMENT_DIRS [EXPERIMENT_DIRS ...]
                        List of local directories to use for experiment files
```

## Entrypoint

The main entry point is `extras/ortoa/benchmark/infrastructure/main.py`

## Jobs

The `ClientJob` class defined in `extras/ortoa/benchmark/infrastructure/jobs.py` implements the `JobProtocol` class. Most importantly, is specifies how a job is run.
