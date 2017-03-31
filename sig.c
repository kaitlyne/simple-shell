#include <stdlib.h>
#include <stdio.h>
#include "sig.h"

void catch_sig(int signum) {
	fprintf(stderr, "%d caught\n", signum);
	exit(signum);
}
