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

#define SEC_IN_MCS 1000000 /* 1 sec = 10^6 us */
#define DIR_DEV "/dev/" /* directory device file */
#define SIZE_PATH 256 /* size (in byte) for path name */
#define N_CHAR_LINE 200 /* size char for one measure */
#define PLUS_CHAR_LINE 5 /* how many more lines have to contain file sparse */ 
#define KERNEL_UNIT_BUFFER 1024 /* kernel buffer size  */
#define N_UNIT 500 /* max size of measure for each cpu, for each read() to device */

/* the hardware type that runs operation */
#define OP_NO_DEF 0 
#define OP_CPU 1
#define OP_DMA 2

/* for estimation of the sparse file size */
#define MAX_SIZE_SP(_n_read , _n_cpu) ((_n_read + PLUS_CHAR_LINE) * N_UNIT * N_CHAR_LINE * _n_cpu) 

/**
 * structure to record a measure
 * @index_cpu_event: index of the cpu used to increase the counter event 
 * @count event: value of count event for measure
 * @addr: return address of copy_to/from_user
 * @size: copied data (byte) 
 * @type_op: the hardware type that runs operation
 * @index_cpu: cpu that finishes the measure
 * @clock_cpu_start: start measure time (ns) 
 * @clock_cpu_end: end measure time (ns) 
*/ 
struct unit_buff_user
{
	unsigned short index_cpu_event; 
	long long count_event; 
  void *addr; 
  unsigned long size; 
  unsigned short type_op; 
  unsigned short index_cpu; 
  unsigned long long clock_cpu_start;
  unsigned long long clock_cpu_end;
};

/**
 * structure to group measures, one for each cpu
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
 * @fd_sp: sparse file descriptor
 * @path_file_sp: path name of sparse file
 * @fd_fl: regular file descriptor
 * @path_file_fl: path name of regular file
 * @size_max: estimation of the sparse file size
 * @offset_map: offset for mmap
 */
struct daemon_trace
{
	int fd_sp; 
	char path_file_sp[SIZE_PATH]; 
	int fd_fl; 
	char path_file_fl[SIZE_PATH]; 
	unsigned long size_max; 
	off_t offset_map;
};

#endif
