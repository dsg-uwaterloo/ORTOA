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


############################################
# Help
############################################

ortoa-help() {
    cat <<'_EOF_'
-------------------------------------------------------------
ortoa-lib: a collection of bash functions to ease development
-------------------------------------------------------------

    Formatters:
        ortoa-clang-format: ------- check staged C++ projects for formatting issues using git-clang-format
        ortoa-clang-format-all: --- check all C++ projects for formatting issues

    Other:
        ortoa-help: --------------- prints this help message

Happy developing!
_EOF_

    [[ ${#} -eq 0 ]]
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

    source scripts/clang-format-all.sh host/ enclave/ crypto/ client/
}
export -f ortoa-clang-format-all
