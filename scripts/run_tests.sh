#!/bin/bash
set -eou pipefail
set -x

run_test() {
    work_dir="$(realpath $1)"

    cmake -S "$1" -B build
    cmake --build build -t test -j 4
    ./build/test
}

run_test hw5
