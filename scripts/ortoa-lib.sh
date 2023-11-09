#!/bin/bash

# Bash library for controlling the ORTOA build and environment

# Assumptions:
#   - repo root does not move

############################################
# Variables
############################################

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# always top-level even in submodule (TODO: bug if more than one submodule deep)
export REPO_ROOT=$(cd ${SCRIPT_DIR} && git rev-parse --show-superproject-working-tree --show-toplevel | head -1)
export BUILD_DIR=${REPO_ROOT}/build

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
        ortoa-simulate: ------------- Run ORTOA in simulation mode
    
    Data Generation:
        ortoa-generate-seed: -------- Seed Data Generation script for ORTOA-tee
        ortoa-generate-operations: -- Operation Generation script for ORTOA-tee 

    Formatters:
        ortoa-clang-format: --------- Check staged C++ files for formatting issues
        ortoa-clang-format-all: ----- Check all C++ projects for formatting issues

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

    ${BUILD_DIR}/src/client/client "${@}"
}
export -f ortoa-client-run


ortoa-simulate() {
    local HELP="""\
Run ORTOA in sumulate mode

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

    ${BUILD_DIR}/src/host/ortoa-host ${BUILD_DIR}/src/enclave/ortoa-enc.signed --simulate
}
export -f ortoa-simulate

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

    if [[ ${#} -ge 1 ]]
    then
        git clang-format "${@}"
    else
        git clang-format ${REPO_ROOT}
    fi
}
export -f ortoa-clang-format


ortoa-clang-format-all() {
    local HELP="""\
Check all C++ projects for formatting issues.

Syntax: ortoa-clang-format [-h]
----------------------------------------------
    -h                  Print this help message
"""
    
    OPTIND=1
    while getopts ":h" option; do
        case "${option}" in
            h) echo "${HELP}"; return 0 ;;
        esac
    done

    source ${REPO_ROOT}/scripts/formatting-and-linting/clang-format-all.sh host/ enclave/ crypto/ client/
}
export -f ortoa-clang-format-all


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
    python3 ${REPO_ROOT}/scripts/data-generation/generate-seed-data.py "${@}"
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
    python3 ${REPO_ROOT}/scripts/data-generation/generate-sample-operations.py "${@}"

}