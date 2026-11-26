// dbg — a small ptrace-based debugger for x86-64 Linux programs.
// Milestone 1: start a program under ptrace, stop at its first instruction,
// show registers, single-step, continue.

#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "debugger.h"

static void usage(const char *prog) {
    fprintf(stderr, "usage: %s <program> [args...]\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child: ask to be traced, then replace ourselves with the target.
        // After execv the kernel stops the child with SIGTRAP before the
        // target's first instruction runs.
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            perror("ptrace(TRACEME)");
            _exit(127);
        }
        execv(argv[1], &argv[1]);
        perror("execv");
        _exit(127);
    }

    struct debugger dbg = { .pid = pid, .running = 1 };
    if (dbg_wait(&dbg) != 0) {
        return 1;
    }
    printf("Started '%s' (pid %d), stopped at entry. Type 'help' for commands.\n",
           argv[1], pid);

    dbg_repl(&dbg);
    return 0;
}
