#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "command.h"
#include "files.h"

int command(int argc, char** argv, int Optind) {
	// Count the number of arguments
	int num_args = 0;
	for (int i = Optind; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			break;
		}
		else {
			num_args++;
		}
	}
	// Check that there are at least four arguments
	if (num_args < 4) {
		fprintf(stderr, "--command: At least 4 arguments required\n");
		return 1;
	}

	// Parse the first three arguments
	long index_in = 0;
	long index_out = 1;
	long index_err = 2;
	char* arg0 = argv[Optind];
	Optind++;
	char* arg1 = argv[Optind];
	Optind++;
	char* arg2 = argv[Optind];
	Optind++;
	char* endptr0 = arg0;
	char* endptr1 = arg1;
	char* endptr2 = arg2;
	// Convert string to long
	index_in = strtol(arg0, &endptr0, 10);
	index_out = strtol(arg1, &endptr1, 10);
	index_err = strtol(arg2, &endptr2, 10);
	// Error check: File number is too high
	if ((index_in >= fd_arr_curindex) || (index_out >= fd_arr_curindex) || (index_err >= fd_arr_curindex)) {
		fprintf(stderr, "--command: Invalid file number\n");
		return 1;
	}
	// Error check: Negative file number
	if ((index_in < 0) || (index_out < 0) || (index_err < 0)) {
		fprintf(stderr, "--command: First three arguments must be nonnegative\n");
		return 1;
	}
	// Error check: First three arguments don't have any numbers
	if ((endptr0 == arg0) || (endptr1 == arg1) || (endptr2 == arg2)) {
		fprintf(stderr, "--command: First three arguments must be nonnegative integers\n");
		return 1;
	}
	// Error check: First three arguments contain non numeric characters
	if ((*endptr0 != '\0') || (*endptr1 != '\0') || (*endptr2 != '\0')) {
		fprintf(stderr, "--command: First three arguments only take numeric characters\n");
		return 1;
	}

	// Parse the rest of the arguments and store them in a new argv
	int command_argv_capacity = num_args - 2;	// num_args - 3 file numbers + 1 null pointer to terminate array
	char** command_argv = (char**)malloc(sizeof(char*)*command_argv_capacity);
	if (command_argv == NULL) {
		fprintf(stderr, "--command: Error allocating memory\n");
		return 1;
	}
	int command_argv_curindex = 0;
	for (int i = Optind; command_argv_curindex < command_argv_capacity-1 && i < argc; i++) {
		command_argv[command_argv_curindex] = argv[i];
		command_argv_curindex++;
	}
	command_argv[command_argv_capacity-1] = NULL;

	int fd_in = fd_arr[index_in];
	int fd_out = fd_arr[index_out];
	int fd_err = fd_arr[index_err];

	// Create a child process
	pid_t pid = fork();
	if (pid == -1) {
		fprintf(stderr, "--command: Error creating child process\n");
		return 1;
	}
	// Child process
	if (pid == 0) {
		// Duplicate file descriptors
		int dup2_in = dup2(fd_in, 0);
		int dup2_out = dup2(fd_out, 1);
		int dup2_err = dup2(fd_err, 2);
		if ((dup2_in == -1) || (dup2_out == -1) || (dup2_err == -1)) {
			fprintf(stderr, "--command: Error duplicating file descriptor\n");
			return 1;
		}
		// Close file descriptors
		for (int i = 0; i < fd_arr_curindex; i++) {
			close(fd_arr[i]);
			fd_arr[i] = -1;
		}
		// Execute command
		if (execvp(command_argv[0], command_argv) < 0) {
			fprintf(stderr, "Error executing --command: %s\n", strerror(errno));
			return 1;
		}
	}
	// Parent process
	else {
		// Not enouch space in child array
		if (child_arr_curindex >= child_arr_capacity) {
			child_arr_capacity *= 2;
			child_struct* tmp = (child_struct*)realloc(child_arr, sizeof(child_struct)*child_arr_capacity);
			if (tmp == NULL) {
				fprintf(stderr, "Error reallocating memory");
				return 1;
			}
			else {
				child_arr = tmp;
			}
		}
		// Store child process info in array
		child_arr[child_arr_curindex].pid = pid;
		child_arr[child_arr_curindex].argv = command_argv;
		child_arr_curindex++;
	}
	return 0;
}
