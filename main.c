#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "files.h"
#include "command.h"
#include "sig.h"

enum OPTIONS {
	Append = 1,
	Cloexec,
	Creat,
	Directory,
	Dsync,
	Excl,
	Nofollow,
	Nonblock,
	Rsync,
	Sync,
	Trunc,
	Rdonly,
	Rdwr,
	Wronly,
	Pipe,
	Command,
	Wait,
	Close,
	Verbose,
	Abort,
	Catch,
	Ignore,
	Default,
	Pause,
	Profile
};

static struct option long_options[] = {
	{"append", no_argument, 0, Append},
	{"cloexec", no_argument, 0, Cloexec},
	{"creat", no_argument, 0, Creat},
	{"directory", no_argument, 0, Directory},
	{"dsync", no_argument, 0, Dsync},
	{"excl", no_argument, 0, Excl},
	{"nofollow", no_argument, 0, Nofollow},
	{"nonblock", no_argument, 0, Nonblock},
	{"rsync", no_argument, 0, Rsync},
	{"sync", no_argument, 0, Sync},
	{"trunc", no_argument, 0, Trunc},
	{"rdonly", required_argument, 0, Rdonly},
	{"rdwr", required_argument, 0, Rdwr},
	{"wronly", required_argument, 0, Wronly},
	{"pipe", no_argument, 0, Pipe},
	{"command", no_argument, 0, Command},
	{"wait", no_argument, 0, Wait},
	{"close", required_argument, 0, Close},
	{"verbose", no_argument, 0, Verbose},
	{"abort", no_argument, 0, Abort},
	{"catch", required_argument, 0, Catch},
	{"ignore", required_argument, 0, Ignore},
	{"default", required_argument, 0, Default},
	{"pause", no_argument, 0, Pause},
	{"profile", no_argument, 0, Profile},
	{0, 0, 0, 0}
};

// Print output for --verbose
void verbose(int long_options_index, char* optarg, int optind, int argc, char** argv) {
	printf("--%s", long_options[long_options_index].name);
	// If option has an argument, print it
	if (optarg) {
		printf(" %s", optarg);
	}
	// Print the rest of the arguments
	for (int i = optind; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			break;
		}
		else {
			printf(" %s", argv[i]);
		}
	}
	printf("\n");
	return;
}

double total_user_time_sec;	// Variable to store total parent user time for --profile
double total_sys_time_sec;	// Variable to store total parent sys time for --profile

// Find and print the time difference between the beginning and end of execution of an option for --profile
void profile_times(struct rusage usage1, struct rusage usage2) {
	const double mil = 1000000;
	// Find user time and sys time at each call of getrusage
	double user_time1 = (double)usage1.ru_utime.tv_sec + (double)(usage1.ru_utime.tv_usec/mil);
	double sys_time1 = (double)usage1.ru_stime.tv_sec + (double)(usage1.ru_stime.tv_usec/mil);
	double user_time2 = (double)usage2.ru_utime.tv_sec + (double)(usage2.ru_utime.tv_usec/mil);
	double sys_time2 = (double)usage2.ru_stime.tv_sec + (double)(usage2.ru_stime.tv_usec/mil);

	// Find the difference in times between each getrusage call
	double user_time_sec = user_time2 - user_time1;
	double sys_time_sec = sys_time2 - sys_time1;

	// Add to the total parent time
	total_user_time_sec += user_time_sec;
	total_sys_time_sec += sys_time_sec;

	// Convert seconds to minutes+seconds
	int user_time_min = 0;
	int sys_time_min = 0;
	if (user_time_sec >= 60) {
		user_time_min = user_time_sec / 60;
		user_time_sec = user_time_sec - (double)(user_time_min*60);
	}
	if (sys_time_sec >= 60) {
		sys_time_min = sys_time_sec / 60;
		sys_time_sec = sys_time_sec - (double)(sys_time_min*60);
	}

	// Output user and sys time
	printf("user\t%dm", user_time_min);
	printf("%fs\n", user_time_sec);
	printf("sys\t%dm", sys_time_min);
	printf("%fs\n", sys_time_sec);

	return;
}

int main(int argc, char** argv)
{
	int ret = 0;	// Return value of the program

	// Allocate memory to store file descriptors
	if (allocate_fd_arr()) {
		ret = 1;
	}

	// Allocate memory to store info about child processes
	child_arr_capacity = 50;
	child_arr_curindex = 0;
	child_arr = (child_struct*)malloc(sizeof(child_struct)*child_arr_capacity);
	if (child_arr == NULL) {
		fprintf(stderr, "Error allocating memory");
		ret = 1;
	}

	int option = 0;
	int long_options_index = 0;
	int verbose_opt = 0;
	int file_flags = 0;
	int wait_opt = 0;
	int profile_opt = 0;
	total_user_time_sec = 0;
	total_sys_time_sec = 0;

	// Parse the options
	while(1) {
		option = getopt_long(argc, argv, "", long_options, &long_options_index);

		// No more options left to parse
		if (option == -1) {
			break;
		}

		// --verbose was used
		if (verbose_opt) {
			verbose(long_options_index, optarg, optind, argc, argv);
		}

		switch(option) {
		case Append:
			file_flags = file_flags | O_APPEND;
			break;
		case Cloexec:
			file_flags = file_flags | O_CLOEXEC;
			break;
		case Creat:
			file_flags = file_flags | O_CREAT;
			break;
		case Directory:
			file_flags = file_flags | O_DIRECTORY;
			break;
		case Dsync:
			file_flags = file_flags | O_DSYNC;
			break;
		case Excl:
			file_flags = file_flags | O_EXCL;
			break;
		case Nofollow:
			file_flags = file_flags | O_NOFOLLOW;
			break;
		case Nonblock:
			file_flags = file_flags | O_NONBLOCK;
			break;
		case Rsync:
			file_flags = file_flags | O_RSYNC;
			break;
		case Sync:
			file_flags = file_flags | O_SYNC;
			break;
		case Trunc:
			file_flags = file_flags | O_TRUNC;
			break;
		case Rdonly: {
			struct rusage usage1;
			struct rusage usage2;
			// Start profiling
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--rdonly getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			// Open file in read-only mode
			file_flags = file_flags | O_RDONLY;
			int fd = open(optarg, file_flags);
			file_flags = 0;
			if (fd < 0) {
				fprintf(stderr, "--rdonly: Error opening read-only file\n");
				ret = 1;
				break;
			}
			// Not enough space in fd array
			if (fd_arr_curindex >= fd_arr_capacity) {
				int realloc_ret = reallocate_fd_arr(fd_arr, fd_arr_capacity);
				if (realloc_ret) {
					ret = 1;
					break;
				}
			}
			// Add file to fd array
			fd_arr[fd_arr_curindex] = fd;
			fd_arr_curindex++;
			// End profiling
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--rdonly getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Rdwr: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--rdwr getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			file_flags = file_flags | O_RDWR;
			int fd = open(optarg, file_flags);
			file_flags = 0;
			if (fd < 0) {
				fprintf(stderr, "--rdwr: Error opening read-and-write-only file\n");
				ret = 1;
				break;
			}
			// Not enough space in fd array
			if (fd_arr_curindex >= fd_arr_capacity) {
				int realloc_ret = reallocate_fd_arr(fd_arr, fd_arr_capacity);
				if (realloc_ret) {
					ret = 1;
					break;
				}
			}
			// Add file to fd array
			fd_arr[fd_arr_curindex] = fd;
			fd_arr_curindex++;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--rdwr getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Wronly: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--wronly getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			// Open file in write-only mode
			file_flags = file_flags | O_WRONLY;
			int fd = open(optarg, file_flags);
			file_flags = 0;
			if (fd < 0) {
				fprintf(stderr, "--wronly: Error opening write-only file\n");
				ret = 1;
				break;
			}
			// Not enough space in fd array
			if (fd_arr_curindex >= fd_arr_capacity) {
				int realloc_ret = reallocate_fd_arr(fd_arr, fd_arr_capacity);
				if (realloc_ret) {
					ret = 1;
					break;
				}
			}
			// Add file to fd array
			fd_arr[fd_arr_curindex] = fd;
			fd_arr_curindex++;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--wronly getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Pipe: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--pipe getrusage: %s", strerror(errno));
					ret = 1;
				}
				
			}
			int fd_pipe_arr[2];
			fd_pipe_arr[0] = -1;
			fd_pipe_arr[1] = -1;
			// Create a pipe
			if (pipe(fd_pipe_arr) == -1) {
				perror("--pipe");
				ret = 1;
				break;
			}
			// Not enouch space in fd array for two file descriptors
			if ((fd_arr_curindex + 1) >= fd_arr_capacity) {
				int realloc_ret = reallocate_fd_arr(fd_arr, fd_arr_capacity);
				if (realloc_ret) {
					ret = 1;
					break;
				}
			}
			// Add file descriptors to fd array
			fd_arr[fd_arr_curindex] = fd_pipe_arr[0];
			fd_arr_curindex++;
			fd_arr[fd_arr_curindex] = fd_pipe_arr[1];
			fd_arr_curindex++;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--pipe getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Command: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--command getrusage: %s", strerror(errno));
					ret = 1;
				}				
			}
			int ret_command = command(argc, argv, optind);
			if (ret_command) {
				ret = 1;
			}
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--command getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Wait:
			wait_opt = 1;
			break;
		case Verbose:
			verbose_opt = 1;
			break;
		case Close: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--close getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			char* endptr = optarg;
			long index_closed = strtol(optarg, &endptr, 10);
			// Error check: File number is too high
			if (index_closed >= fd_arr_curindex) {
				fprintf(stderr, "--close: Invalid file number\n");
				ret = 1;
				break;
			}
			// Error check: Negative file number
			if (index_closed < 0) {
				fprintf(stderr, "--close: File number must be nonnegative\n");
				ret = 1;
				break;
			}
			// Error check: Argument is not a number
			if (endptr == optarg) {
				fprintf(stderr, "--close: File number must be a nonnegative integer\n");
				ret = 1;
				break;
			}
			// Error check: Argument contains a non numeric character
			if (*endptr != '\0') {
				fprintf(stderr, "--close: File number contains a non numeric character\n");
				ret = 1;
				break;
			}

			// Close file descriptor
			int close_status = close(fd_arr[index_closed]);
			if (close_status == -1) {
				fprintf(stderr, "--close: Error closing file descriptor\n");
				ret = 1;
				break;
			}

			// Mark file descriptor as invalid in fd array
			fd_arr[index_closed] = -1;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--close getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Abort: {
			// Dereference a null pointer
			char* nullp = NULL;
			char bad = *nullp;
			break;
		}
		case Catch: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--catch getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			char* endptr = optarg;
			long signum = strtol(optarg, &endptr, 10);
			// Error check: Argument contains a non numeric character
			if (*endptr != '\0') {
				fprintf(stderr, "--catch: Argument must not contain non-numeric characters\n");
				ret = 1;
				break;
			}
			sig_struct.sa_handler = catch_sig;
			// Set up signal handler
			int sigaction_status = sigaction(signum, &sig_struct, NULL);
			if (sigaction_status == -1) {
				fprintf(stderr, "--catch: %s\n", strerror(errno));
				ret = 1;
			}
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--catch getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}	
			break;
		}
		case Ignore: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--ignore getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			char* endptr = optarg;
			long signum = strtol(optarg, &endptr, 10);
			// Error check: Argument contains a non numeric character
			if (*endptr != '\0') {
				fprintf(stderr, "--ignore: Argument must not contain non numeric characters\n");
				ret = 1;
				break;
			}
			// Ignore signal
			if (signal(signum, SIG_IGN) == SIG_ERR) {
				fprintf(stderr, "--ignore: %s\n", strerror(errno));
				ret = 1;
			}
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--ignore getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}	
			break;
		}
		case Default: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--default getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			char* endptr = optarg;
			long signum = strtol(optarg, &endptr, 10);
			// Error check: Argument contains a non numeric character
			if (*endptr != '\0') {
				fprintf(stderr, "--default: Argument must not contain non numeric characters\n");
				ret = 1;
				break;
			}
			// Deal with signal with default behavior
			if (signal(signum, SIG_DFL) == SIG_ERR) {
				fprintf(stderr, "--default: %s\n", strerror(errno));
				ret = 1;
			}
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--default getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Pause: {
			struct rusage usage1;
			struct rusage usage2;
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage1) == -1) {
					fprintf(stderr, "--pause getrusage: %s", strerror(errno));
					ret = 1;
				}
			}
			pause();
			if (profile_opt) {
				if (getrusage(RUSAGE_SELF, &usage2) == -1) {
					fprintf(stderr, "--pause getrusage: %s", strerror(errno));
					ret = 1;
				}
				if (!verbose_opt) {
					verbose(long_options_index, optarg, optind, argc, argv);
				}
				profile_times(usage1, usage2);
			}
			break;
		}
		case Profile:
			profile_opt = 1;
			break;
		}
	}

	// Close all file descriptors
	for (int i = 0; i < fd_arr_curindex; i++) {
		close(fd_arr[i]);
		fd_arr[i] = -1;
	}

	int stat = 0;
	int max_exit_stat = 0;
	int exit_stat = 0;
	int child_user_time_min = 0;
	double child_user_time_sec = 0;
	int child_sys_time_min = 0;
	double child_sys_time_sec = 0;

	if (wait_opt) {
		struct rusage usage1;
		struct rusage usage2;
		// Start profiling for --wait
		if (profile_opt) {
			if (!verbose_opt) {
				verbose(long_options_index, NULL, 0, 0, NULL);
			}
			if (getrusage(RUSAGE_SELF, &usage1) == -1) {
				fprintf(stderr, "--wait getrusage: %s", strerror(errno));
				ret = 1;
			}
		}
		// Wait for each of the child processes
		while (1) {
			pid_t pid_wait = waitpid(-1, &stat, 0);
			// No more child processes to wait for
			if (pid_wait < 0) {
				break;
			}
			// Check if child process exited normally
			if (WIFEXITED(stat)) {
				// Get exit status
				exit_stat = WEXITSTATUS(stat);
				if (exit_stat > max_exit_stat) {
					max_exit_stat = exit_stat;
				}
			}
			// Child process didn't exit normally
			else {
				exit_stat = 1;
				if (exit_stat > max_exit_stat) {
					max_exit_stat = exit_stat;
				}
			}
			// Look for child info struct
			int i = 0;
			for (; i < child_arr_curindex; i++) {
				if (child_arr[i].pid == pid_wait) {
					break;
				}
			}
			if (i >= child_arr_curindex) {
				fprintf(stderr, "--wait: Error with pid");
				ret = 1;
				break;
			}
			// Print exit status
			printf("%d", exit_stat);
			// Print arguments
			char** finished_child_argv = child_arr[i].argv;
			for (int j = 0; finished_child_argv[j] != NULL; j++) {
				printf(" %s", finished_child_argv[j]);
			}
			printf("\n");
		}
		if (profile_opt) {
			// End profiling for --wait
			if (getrusage(RUSAGE_SELF, &usage2) == -1) {
				fprintf(stderr, "--wait getrusage: %s", strerror(errno));
				ret = 1;
			}
			profile_times(usage1, usage2);
			
			// Profile for child processes
			struct rusage child_usage;
			if (getrusage(RUSAGE_CHILDREN, &child_usage) == -1) {
				fprintf(stderr, "getrusage: %s", strerror(errno));
				ret = 1;
			}

			// Find number of minutes and seconds for child processes
			const double mil = 1000000;
			child_user_time_sec = (double)(child_usage.ru_utime.tv_sec) + (double)(child_usage.ru_utime.tv_usec/mil);
			child_sys_time_sec = (double)(child_usage.ru_stime.tv_sec) + (double)(child_usage.ru_stime.tv_usec/mil);
			child_user_time_min = 0;
			child_sys_time_min = 0;
			if (child_user_time_sec >= 60) {
				child_user_time_min = child_user_time_sec / 60;
				child_user_time_sec = child_user_time_sec - (double)(child_user_time_min*60);
			}
			if (child_sys_time_sec >= 60) {
				child_sys_time_min = child_sys_time_sec / 60;
				child_sys_time_sec = child_sys_time_sec - (double)(child_sys_time_min*60);
			}
		}
	}
	if (profile_opt) {
		// Convert total parent time to minutes+seconds
		int total_user_time_min = 0;
		if (total_user_time_sec >= 60) {
			total_user_time_min = total_user_time_sec / 60;
			total_user_time_sec = total_user_time_sec - (double)(total_user_time_min*60);
		}
		int total_sys_time_min = 0;
		if (total_sys_time_sec >= 60) {
			total_sys_time_min = total_sys_time_sec / 60;
			total_sys_time_sec = total_sys_time_sec - (double)(total_sys_time_min*60);
		}

		// Output total parent time
		printf("Total CPU time for parent:\nuser\t%dm", total_user_time_min);
		printf("%fs\n", total_user_time_sec);
		printf("sys\t%dm", total_sys_time_min);
		printf("%fs\n", total_sys_time_sec);

		if (wait_opt) {
			// Output total children time
			printf("Total CPU time for children:\nuser\t%dm", child_user_time_min);
			printf("%fs\n", child_user_time_sec);
			printf("sys\t%dm", child_sys_time_min);
			printf("%fs\n", child_sys_time_sec);
		}
	}
	
	// Free dynamic arrays
	free_fd_arr(fd_arr);
	for (int i = 0; i < child_arr_curindex; i++) {
		free(child_arr[i].argv);
	}
	free(child_arr);

	if (max_exit_stat > ret) {
		ret = max_exit_stat;
	}
	return ret;
}
