#include "main.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
/*
 1) SIGHUP	 2) SIGINT	 3) SIGQUIT	 4) SIGILL	 5) SIGTRAP
 6) SIGABRT	 7) SIGBUS	 8) SIGFPE	 9) SIGKILL	10) SIGUSR1
11) SIGSEGV	12) SIGUSR2	13) SIGPIPE	14) SIGALRM	15) SIGTERM
16) SIGSTKFLT	17) SIGCHLD	18) SIGCONT	19) SIGSTOP	20) SIGTSTP
21) SIGTTIN	22) SIGTTOU	23) SIGURG	24) SIGXCPU	25) SIGXFSZ
26) SIGVTALRM	27) SIGPROF	28) SIGWINCH	29) SIGIO	30) SIGPWR
31) SIGSYS	34) SIGRTMIN	35) SIGRTMIN+1	36) SIGRTMIN+2	37) SIGRTMIN+3
38) SIGRTMIN+4	39) SIGRTMIN+5	40) SIGRTMIN+6	41) SIGRTMIN+7	42) SIGRTMIN+8
43) SIGRTMIN+9	44) SIGRTMIN+10	45) SIGRTMIN+11	46) SIGRTMIN+12	47) SIGRTMIN+13
48) SIGRTMIN+14	49) SIGRTMIN+15	50) SIGRTMAX-14	51) SIGRTMAX-13	52) SIGRTMAX-12
53) SIGRTMAX-11	54) SIGRTMAX-10	55) SIGRTMAX-9	56) SIGRTMAX-8	57) SIGRTMAX-7
58) SIGRTMAX-6	59) SIGRTMAX-5	60) SIGRTMAX-4	61) SIGRTMAX-3	62) SIGRTMAX-2
63) SIGRTMAX-1	64) SIGRTMAX
*/

const char* SIGNAL_NAMES[] = {
    NULL,          "SIGHUP",      "SIGINT",      "SIGQUIT",     "SIGILL",      "SIGTRAP",     "SIGABRT",     "SIGBUS",      "SIGFPE",
    "SIGKILL",     "SIGUSR1",     "SIGSEGV",     "SIGUSR2",     "SIGPIPE",     "SIGALRM",     "SIGTERM",     "SIGSTKFLT",   "SIGCHLD",
    "SIGCONT",     "SIGSTOP",     "SIGTSTP",     "SIGTTIN",     "SIGTTOU",     "SIGURG",      "SIGXCPU",     "SIGXFSZ",     "SIGVTALRM",
    "SIGPROF",     "SIGWINCH",    "SIGIO",       "SIGPWR",      "SIGSYS",      NULL,          NULL,          "SIGRTMIN",    "SIGRTMIN+1",
    "SIGRTMIN+2",  "SIGRTMIN+3",  "SIGRTMIN+4",  "SIGRTMIN+5",  "SIGRTMIN+6",  "SIGRTMIN+7",  "SIGRTMIN+8",  "SIGRTMIN+9",  "SIGRTMIN+10",
    "SIGRTMIN+11", "SIGRTMIN+12", "SIGRTMIN+13", "SIGRTMIN+14", "SIGRTMIN+15", "SIGRTMAX-14", "SIGRTMAX-13", "SIGRTMAX-12", "SIGRTMAX-11",
    "SIGRTMAX-10", "SIGRTMAX-9",  "SIGRTMAX-8",  "SIGRTMAX-7",  "SIGRTMAX-6",  "SIGRTMAX-5",  "SIGRTMAX-4",  "SIGRTMAX-3",  "SIGRTMAX-2",
    "SIGRTMAX-1",  "SIGRTMAX"};

const uint32_t N_OF_SIGNALS = sizeof(SIGNAL_NAMES) / sizeof(SIGNAL_NAMES[0]);

void signal_handler(int signum) {
    printf("Received signal %d (%s)\n", signum, SIGNAL_NAMES[signum]);
}

return_code_t_e child_process(void) {
    printf("Child process created with pid: %d\n", getpid());
    for (unsigned long i = 1; i < N_OF_SIGNALS; ++i) {
        if (SIGNAL_NAMES[i] == NULL || i == SIGKILL || i == SIGSTOP) {
            continue;
        }
        signal(i, signal_handler);
    }
    while (1) {
        sleep(1);
    }

    return OK;
}

return_code_t_e parent_process(options_t options, pid_t child_id) {
    printf("Parent process running with pid: %d\n", getpid());
    printf("Sending all signals to process with pid: %d\n", child_id);
    for (unsigned long i = 1; i < N_OF_SIGNALS; ++i) {
        if (SIGNAL_NAMES[i] == NULL || i == SIGKILL || i == SIGSTOP) {
            continue;
        }
        usleep(1000);
        kill(child_id, i);
    }
    if (options.sleep_duration > 0) {
        sleep(options.sleep_duration);
    }

    kill(child_id, SIGKILL);

    return OK;
}

return_code_t_e run(options_t options) {
    pid_t process_id = fork();

    if (process_id < 0) {
        return ERROR;
    } else if (process_id == 0) {
        child_process();
    } else if (process_id) {
        parent_process(options, process_id);
    }

    return OK;
}
