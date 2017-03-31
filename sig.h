#ifndef SIG_H
#define SIG_H

#include <signal.h>

struct sigaction sig_struct;
void catch_sig(int signum);	// Signal handler

#endif
