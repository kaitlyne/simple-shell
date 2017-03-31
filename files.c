#include <stdlib.h>
#include <stdio.h>
#include "files.h"

int allocate_fd_arr() {
	fd_arr_capacity = 50;
	fd_arr = (int*)malloc(sizeof(int)*fd_arr_capacity);
	if (fd_arr == NULL) {
		fprintf(stderr, "Error allocating memory");
		return 1;
	}
	fd_arr_curindex = 0;
	return 0;
}

int reallocate_fd_arr(int* fd_arr, int fd_arr_capacity) {
	// Reallocate twice the amount of memory
	fd_arr_capacity *= 2;
	int* tmp = (int*)realloc(fd_arr, sizeof(int)*fd_arr_capacity);
	if (tmp == NULL) {
		fprintf(stderr, "Error reallocating memory");
		return 1;
	}
	else {
		fd_arr = tmp;
	}
	return 0;
}

void free_fd_arr(int* fd_arr) {
	free(fd_arr);
	return;
}
