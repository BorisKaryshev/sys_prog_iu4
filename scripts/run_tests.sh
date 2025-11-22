#!/bin/bash
set -eou pipefail
set -x

run_test() {
    work_dir="$(realpath $1)"

    if [ -e $2 ]; then
        cmake -S "$1" -B build
    else
        cmake -S "$1" -B build "$2"
    fi
    cmake --build build -t test -j 4
    ./build/test
}

opts=${1:-""}

run_test hw5 "$opts"
