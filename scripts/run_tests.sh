#!/bin/bash
set -eou pipefail
set -x

run_test() {
    work_dir="$(realpath $1)"
    cd "$work_dir"

    cmake -S . -B build -DBUILD_MODE=DEBUG
    cmake --build build -t test -j 4
    ./build/test
    cd -
}

run_valgrind() {
    work_dir="$(realpath $1)"
    cd "$work_dir"

    cmake -S . -B build -DBUILD_MODE=DEBUG
    cmake --build build -t test -j 4
    valgrind --error-exitcode=1  --leak-check=full --show-leak-kinds=all ./build/test
    echo "Valgrind res is $?"
    cd -
}


# run_test "1_semester/hw5"
# run_valgrind "1_semester/hw5"
#
# run_test "1_semester/hw6"
# run_valgrind "1_semester/hw6"
