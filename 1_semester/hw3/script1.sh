#!/bin/bash

on_death() {
    echo "Process $$ was killed with SIGTERM"
    exit
}

foo() {
    trap on_death TERM

    echo "Hello I am process which runs infinitely"
    # Simulate some work
    count=1
    while true; do
        echo "Working... $count seconds"
        sleep 1
        ((count++))
    done
}


foo &
pid=$!

sleep 4
kill -15 $pid
