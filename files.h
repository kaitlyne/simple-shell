#ifndef FILES_H
#define FILES_H

// Array to store file descriptors
int* fd_arr;
int fd_arr_capacity;
int fd_arr_curindex;

// Allocate the array
int allocate_fd_arr();

// If the array isn't big enough, reallocate more memory
int reallocate_fd_arr(int* fd_arr, int fd_arr_capacity);

// Free memory
void free_fd_arr(int* fd_arr);

#endif
