#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "debugger.h"

int dbg_wait(struct debugger *dbg) {
    int status;
    if (waitpid(dbg->pid, &status, 0) < 0) {
        perror("waitpid");
        dbg->running = 0;
        return -1;
    }
    if (WIFEXITED(status)) {
        printf("Program exited with code %d\n", WEXITSTATUS(status));
        dbg->running = 0;
        return -1;
    }
    if (WIFSIGNALED(status)) {
        printf("Program killed by signal %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));
        dbg->running = 0;
        return -1;
    }
    if (WIFSTOPPED(status)) {
        int sig = WSTOPSIG(status);
        if (sig != SIGTRAP) {
            struct user_regs_struct r;
            ptrace(PTRACE_GETREGS, dbg->pid, NULL, &r);
            printf("Program received signal %d (%s) at 0x%llx\n",
                   sig, strsignal(sig), r.rip);
        }
    }
    return 0;
}

void cmd_regs(struct debugger *dbg) {
    struct user_regs_struct r;
    if (ptrace(PTRACE_GETREGS, dbg->pid, NULL, &r) < 0) {
        perror("ptrace(GETREGS)");
        return;
    }
    printf("rax 0x%016llx  rbx 0x%016llx  rcx 0x%016llx\n", r.rax, r.rbx, r.rcx);
    printf("rdx 0x%016llx  rsi 0x%016llx  rdi 0x%016llx\n", r.rdx, r.rsi, r.rdi);
    printf("rbp 0x%016llx  rsp 0x%016llx  r8  0x%016llx\n", r.rbp, r.rsp, r.r8);
    printf("r9  0x%016llx  r10 0x%016llx  r11 0x%016llx\n", r.r9, r.r10, r.r11);
    printf("r12 0x%016llx  r13 0x%016llx  r14 0x%016llx\n", r.r12, r.r13, r.r14);
    printf("r15 0x%016llx  rip 0x%016llx  eflags 0x%llx\n", r.r15, r.rip, r.eflags);
}

void cmd_step(struct debugger *dbg) {
    if (ptrace(PTRACE_SINGLESTEP, dbg->pid, NULL, NULL) < 0) {
        perror("ptrace(SINGLESTEP)");
        return;
    }
    if (dbg_wait(dbg) == 0) {
        struct user_regs_struct r;
        ptrace(PTRACE_GETREGS, dbg->pid, NULL, &r);
        printf("rip = 0x%llx\n", r.rip);
    }
}

void cmd_continue(struct debugger *dbg) {
    if (ptrace(PTRACE_CONT, dbg->pid, NULL, NULL) < 0) {
        perror("ptrace(CONT)");
        return;
    }
    dbg_wait(dbg);
}

static void help(void) {
    puts("Commands:");
    puts("  regs        show registers");
    puts("  step | s    execute one instruction");
    puts("  cont | c    continue until the program stops or exits");
    puts("  help        show this list");
    puts("  quit | q    kill the program and exit");
}

void dbg_repl(struct debugger *dbg) {
    char line[256];
    while (dbg->running) {
        printf("(dbg) ");
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) {
            break;
        }
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "regs") == 0) {
            cmd_regs(dbg);
        } else if (strcmp(line, "step") == 0 || strcmp(line, "s") == 0) {
            cmd_step(dbg);
        } else if (strcmp(line, "cont") == 0 || strcmp(line, "c") == 0) {
            cmd_continue(dbg);
        } else if (strcmp(line, "help") == 0) {
            help();
        } else if (strcmp(line, "quit") == 0 || strcmp(line, "q") == 0) {
            break;
        } else if (line[0] != '\0') {
            printf("Unknown command '%s'. Type 'help'.\n", line);
        }
    }
    if (dbg->running) {
        ptrace(PTRACE_KILL, dbg->pid, NULL, NULL);
    }
}
