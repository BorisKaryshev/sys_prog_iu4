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

build() {
    work_dir="$(realpath $1)"
    cd "$work_dir"

    cmake -S . -B build -DBUILD_MODE=DEBUG
    cmake --build build -j 4
    cd -
}

buildable=$(echo 1_semester/hw{5,6} 2_semester/hw{1,2,3})
testable=$(echo 1_semester/hw5)

for i in $buildable; do
    build $i
done

for i in $testable; do
    run_test $i
    run_valgrind $i
done
