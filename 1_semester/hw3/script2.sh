#!/bin/bash

print_signal() {
    local sig_num=$1
    local sig_name=$2
    echo "I am process $$ and I catched signal № $sig_num with name $sig_name"

    case "$sig_num" in
        2|15) exit
        ;;
        *) echo default
        ;;
    esac

}

# Trap all signals according to your OS signal mapping
trap 'print_signal 1 SIGHUP' HUP
trap 'print_signal 2 SIGINT' INT
trap 'print_signal 3 SIGQUIT' QUIT
trap 'print_signal 4 SIGILL' ILL
trap 'print_signal 5 SIGTRAP' TRAP
trap 'print_signal 6 SIGABRT' ABRT
trap 'print_signal 7 SIGBUS' BUS
trap 'print_signal 8 SIGFPE' FPE
# SIGKILL (9) cannot be trapped - removed
trap 'print_signal 10 SIGUSR1' USR1
trap 'print_signal 11 SIGSEGV' SEGV
trap 'print_signal 12 SIGUSR2' USR2
trap 'print_signal 13 SIGPIPE' PIPE
trap 'print_signal 14 SIGALRM' ALRM
trap 'print_signal 15 SIGTERM' TERM
trap 'print_signal 16 SIGSTKFLT' STKFLT
trap 'print_signal 17 SIGCHLD' CHLD
trap 'print_signal 18 SIGCONT' CONT
# SIGSTOP (19) cannot be trapped - removed
trap 'print_signal 20 SIGTSTP' TSTP
trap 'print_signal 21 SIGTTIN' TTIN
trap 'print_signal 22 SIGTTOU' TTOU
trap 'print_signal 23 SIGURG' URG
trap 'print_signal 24 SIGXCPU' XCPU
trap 'print_signal 25 SIGXFSZ' XFSZ
trap 'print_signal 26 SIGVTALRM' VTALRM
trap 'print_signal 27 SIGPROF' PROF
trap 'print_signal 28 SIGWINCH' WINCH
trap 'print_signal 29 SIGIO' IO
trap 'print_signal 30 SIGPWR' PWR
trap 'print_signal 31 SIGSYS' SYS

# Simulate some work
count=1
while true; do
    echo "Working... $count seconds"
    sleep 1
    ((count++))
done
