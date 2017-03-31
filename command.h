#ifndef COMMAND_H
#define COMMAND_H

#include <sys/types.h>

typedef struct {
	pid_t pid;
	char** argv;
} child_struct;

child_struct* child_arr;
int child_arr_curindex;
int child_arr_capacity;

// Execute the --command option
int command(int argc, char** argv, int Optind);

#endif
