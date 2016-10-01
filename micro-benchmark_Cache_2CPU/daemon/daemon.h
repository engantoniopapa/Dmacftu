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

#define DIR_DEV "/dev/micro-benchmark_ch" /* directory device file */
#define N_CHAN 4 /* number of device */ 
#define SIZE_PATH 256 /* size (in byte) for path name */
#define N_CHAR_LINE 200 /* size char for one measure */
#define _MAX_BYTE 268435456 /* max size in byte */ 
#define SEC_TO_NS 1000000000 /* 1 sec in nsec */
#define SIZE_REG_MEM  268435456 /* size of thread memory region */
#define ID_CPU 0 /* id cpu for benchmark */

/* type operation to measure */
#define TYPE_MEASURE_FROM 1
#define TYPE_MEASURE_TO 2
#define TYPE_MEASURE_FROM_TO 3


/* state of thread */
#define TH_STATE_OK 0
#define TH_STATE_INIT 1
#define TH_STATE_FAIL -1

/* Pthread Condition Variables & Mutex */
pthread_mutex_t queue_lock; 
pthread_cond_t  queue_full;
short int count_queue;
short int end_pthread;

/**
 * main descriptor thread, 
 * @id_thread: thread id,
 * @id_chan: chan associated with the thread
 * @state_thread: state thread,
 * @counter_op: number of read() & write()
 * @fd_dev: device file descriptor
 * @path_dev: path of @path_dev
 * @region: region memory of thread
 * @size_region: size of @region
 * @size_message: size of memory message 
 */
struct thread_benchmark
{
	pthread_t id_thread;
	short int id_chan;
	short int state_thread;
	unsigned long long counter_op;
	int fd_dev;
	char path_dev[SIZE_PATH];
	char *region;
	unsigned long size_region;
	unsigned long size_message; 
};

#endif
