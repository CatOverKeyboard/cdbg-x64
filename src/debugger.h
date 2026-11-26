#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <sys/types.h>

struct debugger {
    pid_t pid;    // traced process
    int running;  // 1 while the process exists
};

// Wait for the traced process to stop or exit and report why.
// Returns 0 if it stopped, -1 if it exited or an error happened.
int dbg_wait(struct debugger *dbg);

// Read commands from stdin until 'quit' or the program exits.
void dbg_repl(struct debugger *dbg);

// Commands
void cmd_regs(struct debugger *dbg);
void cmd_step(struct debugger *dbg);
void cmd_continue(struct debugger *dbg);

#endif
