#ifndef _MIBENCH_H_
#define _MIBENCH_H_

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
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <time.h>
#include <pthread.h>

#define N_ARG 4
#define SIZE_ARG 150
#define SIZE_PATH 256 /* size (in byte) for path name */
#define N_CHAR_LINE 200 /* size char for one measure */
#define SEC_TO_NS 1000000000 /* 1 sec in nsec */
#define MICROSEC_TO_NS 1000 /* 1 usec in nsec */
#define SLEEP_TIME 100000 /* time sleep in usec between measures */
#define ID_CPU 1 /* id cpu for mibench */

/* type file results */
#define TYPE_FILE_MEASURE 0 
#define TYPE_FILE_SYNTHESIS 1


/* state of thread */
#define TH_STATE_OK 0
#define TH_STATE_INIT 1
#define TH_STATE_FAIL -1

/**
 * measures of MiBench 
 * @time_start: time in ns (start test)
 * @time_end: time in ns (end test)
 * @cache_ref: accesses in last cache level
 * @cache_miss: misses in last cache level
 */
struct mi_result
{
	 unsigned long long time_start;
	 unsigned long long time_end;
	 unsigned long long cache_ref;  
	 unsigned long long cache_miss;
};

/**
 * main descriptor  
 * @results: buffer for test results 
 * @size: size of @results
 * @fd_measure: meausure file descriptor
 * @fd_synthesis: synthesis file descriptor
 * @id_thread: thread id
 * @state_thread: state thread,
 */
struct mibench_state
{
	struct mi_result *results;
	unsigned int size;
	int fd_measure;
	int fd_synthesis;
	pthread_t id_thread;
	short int state_thread;
};

extern void start_susan(int argc, char argv[N_ARG][SIZE_ARG] ,struct mi_result *result);
extern void start_qsort_small(int argc, char argv[N_ARG][SIZE_ARG] ,struct mi_result *result);
extern void start_qsort_large(int argc, char argv[N_ARG][SIZE_ARG] ,struct mi_result *result);

extern long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags);

extern int init_test(struct mibench_state* mi_state);
extern int data_synthesis(struct mibench_state* mi_state , unsigned long long *time_diff , double *per_cache_miss, char *name_test);

#endif
