#ifndef _DAEMON_H_
#define _DAEMON_H_

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <time.h>
#include "../driver/ioctl_mb.h"

#define DIR_DEV "/dev/micro-benchmark_ch" /* directory device file */
#define N_CHAN 4 /* number of device */ 
#define SIZE_PATH 256 /* size (in byte) for path name */
#define N_CHAR_LINE 200 /* size char for one measure */
#define _1MB_TO_BYTE 1048576 /* 1MB in byte */ 
#define _2MB_TO_BYTE 2097152 /* 2MB in byte */ 
#define _1KB_TO_BYTE 1024 /* 1 KB in byte */
#define SEC_TO_NS 1000000000 /* 1 sec in nsec */
#define N_CPU 4 /* number of CPU */

/* type operation to measure */
#define TYPE_MEASURE_FROM 0
#define TYPE_MEASURE_TO 1
#define TYPE_MEASURE_FROM_TO 2


#define DIR_RESULT_FROM_TO "../risultati/measures_from_to.dat"
#define DIR_RESULT_FROM "../risultati/measures_from.dat"
#define DIR_RESULT_TO "../risultati/measures_to.dat"


/* state of thread */
#define TH_STATE_OK 0
#define TH_STATE_INIT 1
#define TH_STATE_FAIL -1

/* Pthread Condition Variables & Mutex */
pthread_mutex_t queue_lock; 
pthread_cond_t  queue_full;
short int count_queue;

/**
 * main descriptor thread, 
 * @id_thread: thread id,
 * @id_chan: chan associated with the thread
 * @state_thread: state thread,
 * @fd_dev: device file descriptor
 * @path_dev: path of @path_dev
 * @time_start: time in ns (start test)
 * @time_end: time in ns (end test)
 * @buf: buffer for test
 * @size: size of @buf
 * @n_op: #operation
 */
struct thread_benchmark
{
	pthread_t id_thread;
	short int id_chan;
	short int state_thread;
	int fd_dev;
	char path_dev[SIZE_PATH];
	unsigned long long time_start;
	unsigned long long time_end;
	char *buf;
	unsigned long size; 
	unsigned long n_op; /* #operation */
};

#endif
