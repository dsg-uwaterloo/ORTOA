#!/bin/bash

REPO_ROOT=$(git rev-parse --show-toplevel)
BENCHMARK_TEST_DIR="${REPO_ROOT}/extras/ortoa/benchmark/test"

run_unit_tests() {
    python3 -m pytest \
        "${BENCHMARK_TEST_DIR}"
}
