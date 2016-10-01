#ifndef _CORE_DAEMON_H_
#define _CORE_DAEMON_H_

#include "daemon.h"
void *start_thread_from_to(void *data);

void *start_thread_from(void *data);

void *start_thread_to(void *data);

int start_micro_benchmark(struct thread_benchmark *th_data , unsigned long max_size ,
				void *(*func_copy)(void *data) );

#endif
