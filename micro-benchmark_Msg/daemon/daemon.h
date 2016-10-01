#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>

#define DIR_DEV "/dev/" /* directory device file */
#define SIZE_PATH 256 /* size (in byte) for path name */
#define N_CHAR_LINE 150 /* size char for one measure */
#define N_UNIT 300 /* max size of measure for each read() to device */
#define _2MB_TO_BYTE 2097152 /* 2MB in byte */ 

#define ID_MEASURE 0 /* id for measure file */
#define ID_SYNTH 1 /* id for synthesis file */

/**
 * structure to record a measure
 * @index_cpu: index of the cpu for measure
 * @size: copied data (byte) 
 * @clock_cpu_start: start measure time (ns) 
 * @clock_cpu_end: end measure time (ns) 
*/ 
struct unit_buff_user
{
	unsigned short index_cpu; 
  unsigned long size; 
  unsigned long long clock_cpu_start; 
  unsigned long long clock_cpu_end; 
};

/**
 * structure to group measures, for size of copy memory
 * @count: number of the measures
 * @buffer: vector of the measures
 */
struct region_mem
{
	unsigned int count;
	struct unit_buff_user buffer[N_UNIT]; 
};

/**
 * main descriptor, 
 * @fd_fl: regular file descriptor for measure
 * @fd_synth: regular file descriptor foo synthesis
 * @path_file_fl: path name of @fd_fl
 * @path_file_synth: path name of @fd_synth
 * 
 */
struct daemon_benchmark
{
	int fd_fl;
	int fd_synth;
	char path_file_fl[SIZE_PATH]; 
	char path_file_synth[SIZE_PATH]; 
};

#endif
