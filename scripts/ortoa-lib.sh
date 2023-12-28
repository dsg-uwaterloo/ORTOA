#!/bin/bash

# Bash library for controlling the ORTOA build and environment

# Assumptions:
#   - repo root does not move

############################################
# Variables
############################################

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# always top-level even in submodule
export REPO_ROOT=$(cd ${SCRIPT_DIR} && git rev-parse --show-superproject-working-tree --show-toplevel | head -1)

export ORTOA_SHARED="${REPO_ROOT}"
export BUILD_DIR="${ORTOA_SHARED}/build"
export INSTALL_DIR="${ORTOA_SHARED}/install"
export SDK_DIR="${ORTOA_SHARED}/extras"

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib:${REPO_ROOT}/install/lib"

############################################
# Help
############################################

ortoa-help() {
    cat <<'_EOF_'
-------------------------------------------------------------
ortoa-lib: a collection of bash functions to ease development
-------------------------------------------------------------

    Running ORTOA:
        ortoa-client-run: ----------- Run the ORTOA client
        ortoa-host: ----------------- Run the ORTOA host
        ortoa-simulate: ------------- Run the ORTOA host in simulation mode
    
    Benchmarking ORTOA:
        ortoa-benchmark: ------------ Benchmark ORTOA with configured experiments
    
    Testing ORTOA:
        ortoa-test-python: ---------- Run pytest on python targets
    
    Data Generation:
        ortoa-generate-seed: -------- Seed Data Generation script for ORTOA-tee
        ortoa-generate-operations: -- Operation Generation script for ORTOA-tee 
    
    Building and Installing:
        ortoa-configure: ------------ Configure C++ projects
        ortoa-build: ---------------- Build C++ projects
        ortoa-install: -------------- Install C++ projects
        ortoa-cbi: ------------------ Configure, build & install C++ projects
        ortoa-clean: ---------------- Cleanup C++ build and install directories

    Formatters:
        ortoa-clang-format: --------- Check staged C++ files for formatting issues
        ortoa-format-python: -------- Format all the python files
        ortoa-sort-python: ---------- Sort the imports in python files
        ortoa-typecheck-python: ----- Typecheck the python files

    Other:
        ortoa-help: ----------------- Prints this help message

Happy developing!
_EOF_

    [[ ${#} -eq 0 ]]
}


############################################
# Running ORTOA
############################################

ortoa-client-run() {
    local HELP="""\
Run the ORTOA client

Syntax: ortoa-client-run [-h]
----------------------------------------------
    -h                  Print this help message
"""

    "${INSTALL_DIR}"/bin/client "${@}"
}

ortoa-simulate() {
    local HELP="""\
Run the ORTOA host in simulate mode

Syntax: ortoa-simulate [-h]
----------------------------------------------
    -h                  Print this help message
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done

    "${INSTALL_DIR}"/bin/ortoa-host ${BUILD_DIR}/src/enclave/ortoa-enc.signed --simulate
}

ortoa-host() {
        local HELP="""\
Run the ORTOA host

Syntax: ortoa-simulate [-h]
----------------------------------------------
    -h                  Print this help message
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done

    "${INSTALL_DIR}"/bin/ortoa-host ${BUILD_DIR}/src/enclave/ortoa-enc.signed
}

############################################
# Benchmarking
############################################

ortoa-benchmark() {
    local HELP="""\
usage: main.py [-h] -e EXPERIMENTS [EXPERIMENTS ...] [-d EXPERIMENT_DIRS [EXPERIMENT_DIRS ...]] [-w WORKING_DIR] [-m MAX_PROCESSES]

options:
  -h, --help            show this help message and exit
  -w WORKING_DIR, --working-dir WORKING_DIR
                        Directory to use as base for experiment directory tree (default: /Users/adrian/projects/ORTOA/benchmark-2023-11-05)
  -m MAX_PROCESSES, --max-processes MAX_PROCESSES
                        Maximum number of processes to use when running experiments (default: None)

Experiments:
  Options to control experiments selected for compilation

  -e EXPERIMENTS [EXPERIMENTS ...], --experiments EXPERIMENTS [EXPERIMENTS ...]
                        List of experiments to compile (experiment name should match zoo object)
  -d EXPERIMENT_DIRS [EXPERIMENT_DIRS ...], --experiment-dirs EXPERIMENT_DIRS [EXPERIMENT_DIRS ...]
                        List of local directories to use for experiment files
"""

    python3 "${REPO_ROOT}/extras/ortoa/benchmark/infrastucture/main.py" "${@}"
}


# Testing
############################################

ortoa-test-python() {
    local HELP="""\
Run ORTOA python tests

Syntax: ortoa-test-python [-h]
----------------------------------------------
    -h                  Print this help message
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done

    source "${REPO_ROOT}/scripts/test/run_benchmark_tests.sh"
    run_unit_tests
}


############################################
# Formatting and linting
############################################

ortoa-clang-format() {
    local HELP="""\
Check staged C++ projects for formatting issues using git-clang-format. 
If DIRECTORY is specified, should only target specified directories.

Syntax: ortoa-clang-format [-h] [DIRECTORY]...
----------------------------------------------
    -h                  Print this help message
    DIRECTORY           Directories to target
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done

    clang-format -i --style=file \
        "${REPO_ROOT}"/src/client/*.h \
        "${REPO_ROOT}"/src/client/*.cpp \
        "${REPO_ROOT}"/src/enclave/*.h \
        "${REPO_ROOT}"/src/enclave/*.cpp \
        "${REPO_ROOT}"/src/host/*.h \
        "${REPO_ROOT}"/src/host/*.cpp \
        "${REPO_ROOT}"/src/libcommon/**/*.h \
        "${REPO_ROOT}"/src/libcommon/**/*.cpp \
        "${REPO_ROOT}"/src/libstorage/**/*.h \
        "${REPO_ROOT}"/src/libstorage/**/*.cpp
}


ortoa-format-python() {
    local HELP="""\
Formats all python files in the extras/ directory

Syntax: ortoa-sort-python [-h]
------------------------------
    -h               Print this help message
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done

    black extras/
}

ortoa-sort-python() {
    local HELP="""\
Sort the imports in all python files in the extras/ directory

Syntax: ortoa-sort-python [-h]
------------------------------
    -h               Print this help message
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done

    isort extras/
}

ortoa-typecheck-python() {
    local HELP="""\
Typechecks the extras/ directory

Syntax: ortoa-typecheck-python [-h]
------------------------------
    -h               Print this help message
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done
    
    pyright -p "${SDK_DIR}" --warnings
}

############################################
# Data Generation
############################################

ortoa-generate-seed() {
    local HELP="""\
usage: Seed Data Generation script for ORTOA-tee [-h] [-o OUTPUT_FILE] [-n N_DATA_POINTS]

Script to generate some seed data the ortoa-tee project

optional arguments:
  -h, --help            show this help message and exit
  -o OUTPUT_FILE, --output_file OUTPUT_FILE
                        File into which to write the sample seed data.
  -n N_DATA_POINTS, --n_data_points N_DATA_POINTS
                        Number of data points to generate.
"""

    python3 ${REPO_ROOT}/extras/data_generation/generate_seed_data.py "${@}"
}

ortoa-generate-operations() {
    local HELP="""\
usage: Operation Generation script for ORTOA-tee [-h] -i INPUT_FILE [-o OUTPUT_FILE] [-n N_OPERATIONS] [-p P_GET]

Script to generate some operations from a seed file

optional arguments:
  -h, --help            show this help message and exit
  -i INPUT_FILE, --input_file INPUT_FILE
                        Input file. This should be a Path to a csv of generated seed data.
  -o OUTPUT_FILE, --output_file OUTPUT_FILE
                        File into which to write the operations.
  -n N_OPERATIONS, --n_operations N_OPERATIONS
                        Number of operations to generate.
  -p P_GET, --p_get P_GET
                        Probability of a GET request. 1-p_get = p_put (probability of a PUT request).
"""

    python3 ${REPO_ROOT}/extras/data_generation/generate_sample_operations.py "${@}"
}

############################################
# Building and Installing
############################################

ortoa-configure() {
    local HELP="""\
Run cmake configuration stage for C++ projects

Syntax: ortoa-configure [-h] [cmake-parameters]
-----------------------------------------------
    h                   Prints this help message
    cmake-parameters    Parameters passed to CMake configure invocation
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
            *) break
        esac
    done

    cd "${REPO_ROOT}"

    shift $((OPTIND - 1))

    mkdir -p "${BUILD_DIR}"
    cmake -S "${REPO_ROOT}" \
          -B "${BUILD_DIR}" \
          -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
          "${@}"
}

ortoa-build() {
    local HELP="""\
Build C++ projects (requires ortoa-configure)

Syntax: ortoa-build [-h] [cmake-parameters]
-------------------------------------------
    h                   Prints this help message
    cmake-parameters    Parameters passed to CMake invocation
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
            *) break ;;
        esac
    done

    cd "${REPO_ROOT}"

    cmake --build "${BUILD_DIR}" "${@}"
}

ortoa-install() {
    local HELP="""\
Install C++ projects (requires ortoa-build)
Syntax: ortoa-install [-h] [cmake-parameters]
---------------------------------------------
    h                   Prints this help message
    cmake-parameters    Parameters passed to CMake invocation
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
            *) break ;;
        esac
    done

    cd "${REPO_ROOT}"

    cmake --install "${BUILD_DIR}" --prefix "${INSTALL_DIR}" "${@}"
}

ortoa-cbi() {
    local HELP="""\
Run cmake configure, build and install stages for C++ projects
Must be run from the repo root

Syntax: ortoa-cbi [-h] [cmake-parameters]
-----------------------------------------
    h                   Prints this help message
    cmake-parameters    Parameters passed to CMake configure invocation
""" 
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
            *) break ;;
        esac
    done

    cd "${REPO_ROOT}"

    ortoa-configure "${@}"
    ortoa-build
    ortoa-install
}

ortoa-clean() {
    local HELP="""\
Clean build and install directories

Syntax: ortoa-clean [-h]
------------------------
    h                   Prints this help message
"""
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
            *) break ;;
        esac
    done

    cd "${REPO_ROOT}"

    rm -rf \
        "${BUILD_DIR}/"* \
        "${INSTALL_DIR}/"*
}
