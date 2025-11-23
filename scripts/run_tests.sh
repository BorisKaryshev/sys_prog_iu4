#!/bin/bash
set -eou pipefail
set -x

run_test() {
    work_dir="$(realpath $1)"
    cd "$work_dir"

    if [ -e $2 ]; then
        cmake -S . -B build
    else
        cmake -S . -B build "$2"
    fi
    cmake --build build -t test -j 4
    ./build/test

    cd -
}

opts=${1:-""}

run_test hw5 "$opts"
run_test hw6 "$opts"
