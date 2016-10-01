#include "mibench.h"

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}

/** 
 * write first row in a file, the # is used for gnuplot 
 * @fd: file descriptor
 * @type: type file measure or synthesis
 * return: zero on success or negative value for error
 */	
int static write_first_row(int fd , short int type)
{	
	char line_buf[N_CHAR_LINE];
	int ret;
	int len;
	
	memset(line_buf, '\0' , N_CHAR_LINE);
	
	if(type == TYPE_FILE_MEASURE)
	{
		strncpy(line_buf , "#              type_test        StartTime___(ns)          EndTime___(ns)"
			"         DiffTime(ns)      Cache_Ref     Cache_Miss     %Cache_Miss\n", N_CHAR_LINE );
	}
	
	if(type == TYPE_FILE_SYNTHESIS)
	{
		strncpy(line_buf , "#              type_test    n_op         AVG_DiffTime(us)"
		"        Max_Diff_Time(us)       AVG_Cache_Ref AVG_%Cache_Miss\n", N_CHAR_LINE);
	}
	
	len = strlen(line_buf);
	
	while( len != 0)
	{
		ret = write(fd, line_buf , len);
		if(ret < 0)
		{
			perror("error write file"); 
			return -EFAULT ;
		}
		
		len = len - ret;
	}
	return 0;
}

/**
 * start test with susan MiBench
 * @argv: argv for susan MiBench
 * @mi_state: main descriptor for test
 * @name_test: test name
 * return: zero on success or negative value for error
*/
int start_test_susan(char argv[N_ARG][SIZE_ARG] , struct mibench_state* mi_state , 
							char *name_test)
{
	struct timespec tspec_start , tspec_end;
	struct mi_result *tmp_result;
	int i ;
	char line_buf[N_CHAR_LINE];
	int len;
	int ret;
	unsigned long long *time_diff;
	double *per_cache_miss;
	
	time_diff = (unsigned long long *) malloc (mi_state->size * sizeof(unsigned long long));
	if(!time_diff)
	{
		perror("error memory"); 
		ret = -ENOMEM;
		goto error_test1;
	}	
	
	per_cache_miss = (double *) malloc (mi_state->size * sizeof(double));
	if(!per_cache_miss)
	{
		perror("error memory"); 
		ret = -ENOMEM;
		goto error_test2;
	}
	
	start_susan(4, argv , &(mi_state->results[0])); //for cache

	for( i = 0 ; i < mi_state->size ; ++i)
	{
		asm volatile("mfence":::"memory");	
		  ret = clock_gettime( CLOCK_REALTIME , &tspec_start );		
		asm volatile("mfence":::"memory");		

			if ( ret != 0)
			{
				perror ("clock_gettime():");
				goto error_test3;
			}

		start_susan(4, argv , &(mi_state->results[i]));
		
		asm volatile("mfence":::"memory");	
			ret = clock_gettime(CLOCK_REALTIME, &tspec_end );	
		asm volatile("mfence":::"memory");		

			if (ret != 0)
			{
				perror ("clock_gettime():");
				goto error_test3;
			}

		tmp_result = &(mi_state->results[i]);
		tmp_result->time_start = tspec_start.tv_sec;
		tmp_result->time_start = (tmp_result->time_start * SEC_TO_NS) + tspec_start.tv_nsec;
	
		tmp_result->time_end = tspec_end.tv_sec;
		tmp_result->time_end = (tmp_result->time_end * SEC_TO_NS) + tspec_end.tv_nsec;
		
		time_diff[i] = tmp_result->time_end - tmp_result->time_start;
		per_cache_miss[i] = (((double) tmp_result->cache_miss) / tmp_result->cache_ref)*100;
		
		usleep(SLEEP_TIME);
	}

	for( i = 0 ; i < mi_state->size ; ++i)
	{
		tmp_result = &(mi_state->results[i]);

		memset(line_buf, '\0' , N_CHAR_LINE);
		sprintf(line_buf," %23s%24llu%24llu%21llu%15llu%15llu%16.6f\n", 
		name_test, tmp_result->time_start , tmp_result->time_end , time_diff[i] , 
		tmp_result->cache_ref, tmp_result->cache_miss , per_cache_miss[i]);

		len = strlen(line_buf);
		while( len != 0)
		{
			ret = write(mi_state->fd_measure, line_buf , len);
			if(ret < 0)
			{
				perror("Error: write file"); 
				goto error_test3;
			}
			len = len - ret;
		}
	}
	
	/* min, max, avg, var */
	ret = data_synthesis(mi_state, time_diff , per_cache_miss , name_test);
	if(ret < 0)
	{
		printf("Error: data synthesis \n");
		goto error_test3;
	}

	free(per_cache_miss);
	free(time_diff);
	return 0;

error_test3:	
	free(per_cache_miss);
	
error_test2:
	free(time_diff);

error_test1:
	return ret;
}

/**
 * start test with qsort small MiBench
 * @argv: argv for qsort small MiBench
 * @mi_state: main descriptor for test
 * @name_test: test name
 * return: zero on success or negative value for error
*/
int start_test_qsort_s(char argv[N_ARG][SIZE_ARG] , struct mibench_state* mi_state , 
							char *name_test)
{
	struct timespec tspec_start , tspec_end;
	struct mi_result *tmp_result;
	int i ;
	char line_buf[N_CHAR_LINE];
	int len;
	int ret;
	unsigned long long *time_diff;
	double *per_cache_miss;
	
	time_diff = (unsigned long long *) malloc (mi_state->size * sizeof(unsigned long long));
	if(!time_diff)
	{
		perror("error memory"); 
		ret = -ENOMEM;
		goto error_test1;
	}	
	
	per_cache_miss = (double *) malloc (mi_state->size * sizeof(double));
	if(!per_cache_miss)
	{
		perror("error memory"); 
		ret = -ENOMEM;
		goto error_test2;
	}
	
	start_qsort_small(2, argv , &(mi_state->results[0])); //for cache

	for( i = 0 ; i < mi_state->size ; ++i)
	{
		asm volatile("mfence":::"memory");	
			ret = clock_gettime( CLOCK_REALTIME , &tspec_start );		
		asm volatile("mfence":::"memory");		

			if (ret != 0)
			{
				perror ("clock_gettime():");
				goto error_test3;
			}

		start_qsort_small(2, argv , &(mi_state->results[i]));

		asm volatile("mfence":::"memory");		
			ret = clock_gettime(CLOCK_REALTIME, &tspec_end );		
		asm volatile("mfence":::"memory");		

			if ( ret != 0)
			{
				perror ("clock_gettime():");
				goto error_test3;
			}

		tmp_result = &(mi_state->results[i]);
		tmp_result->time_start = tspec_start.tv_sec;
		tmp_result->time_start = (tmp_result->time_start * SEC_TO_NS) + tspec_start.tv_nsec;
	
		tmp_result->time_end = tspec_end.tv_sec;
		tmp_result->time_end = (tmp_result->time_end * SEC_TO_NS) + tspec_end.tv_nsec;
		
		time_diff[i] = tmp_result->time_end - tmp_result->time_start;
		per_cache_miss[i] = (((double) tmp_result->cache_miss) / tmp_result->cache_ref)*100;
		
		usleep(SLEEP_TIME);
	}

	for( i = 0 ; i < mi_state->size ; ++i)
	{
		tmp_result = &(mi_state->results[i]);

		memset(line_buf, '\0' , N_CHAR_LINE);
		sprintf(line_buf," %23s%24llu%24llu%21llu%15llu%15llu%16.6f\n", 
		name_test, tmp_result->time_start , tmp_result->time_end , time_diff[i] , 
		tmp_result->cache_ref, tmp_result->cache_miss , per_cache_miss[i]);

		len = strlen(line_buf);
		while( len != 0)
		{
			ret = write(mi_state->fd_measure, line_buf , len);
			if(ret < 0)
			{
				perror("Error: write file"); 
				goto error_test3;
			}
			len = len - ret;
		}
	}
	
	/* min, max, avg, var */
	ret = data_synthesis(mi_state , time_diff , per_cache_miss , name_test);
	if(ret < 0)
	{
		printf("Error: data synthesis \n");
		goto error_test3;
	}
	
	free(per_cache_miss);
	free(time_diff);
	return 0;

error_test3:	
	free(per_cache_miss);
	
error_test2:
	free(time_diff);

error_test1:
	return -1;
}

/**
 * start test with qsort large MiBench
 * @argv: argv for qsort large MiBench
 * @mi_state: main descriptor for test
 * @name_test: test name
 * return: zero on success or negative value for error
*/
int start_test_qsort_l(char argv[N_ARG][SIZE_ARG] , struct mibench_state* mi_state , 
							char *name_test)
{
	struct timespec tspec_start , tspec_end;
	struct mi_result *tmp_result;
	int i ;
	char line_buf[N_CHAR_LINE];
	int len;
	int ret;
	unsigned long long *time_diff;
	double *per_cache_miss;
	
	time_diff = (unsigned long long *) malloc (mi_state->size * sizeof(unsigned long long));
	if(!time_diff)
	{
		perror("error memory"); 
		ret = -ENOMEM;
		goto error_test1;
	}	
	
	per_cache_miss = (double *) malloc (mi_state->size * sizeof(double));
	if(!per_cache_miss)
	{
		perror("error memory");
		ret = -ENOMEM; 
		goto error_test2;
	}
	
	start_qsort_large(2, argv , &(mi_state->results[0])); //for cache

	
	for( i = 0 ; i < mi_state->size ; ++i)
	{
	asm volatile("mfence":::"memory");
		ret = clock_gettime( CLOCK_REALTIME , &tspec_start );		
	asm volatile("mfence":::"memory");		
	
		if ( ret != 0)
		{
			perror ("clock_gettime():");
			goto error_test3;
		}		
	
		start_qsort_large(2, argv , &(mi_state->results[i]));

	asm volatile("mfence":::"memory");	
		ret = clock_gettime(CLOCK_REALTIME, &tspec_end );
	asm volatile("mfence":::"memory");		
	
		if ( ret != 0)
		{
			perror ("clock_gettime():");
			goto error_test3;
		}	

		tmp_result = &(mi_state->results[i]);
		tmp_result->time_start = tspec_start.tv_sec;
		tmp_result->time_start = (tmp_result->time_start * SEC_TO_NS) + tspec_start.tv_nsec;
	
		tmp_result->time_end = tspec_end.tv_sec;
		tmp_result->time_end = (tmp_result->time_end * SEC_TO_NS) + tspec_end.tv_nsec;
		
		time_diff[i] = tmp_result->time_end - tmp_result->time_start;
		per_cache_miss[i] = (((double) tmp_result->cache_miss) / tmp_result->cache_ref)*100;
		
		usleep(SLEEP_TIME);
	}

	for( i = 0 ; i < mi_state->size ; ++i)
	{
		tmp_result = &(mi_state->results[i]);

		memset(line_buf, '\0' , N_CHAR_LINE);
		sprintf(line_buf," %23s%24llu%24llu%21llu%15llu%15llu%16.6f\n", 
		name_test, tmp_result->time_start , tmp_result->time_end , time_diff[i] , 
		tmp_result->cache_ref, tmp_result->cache_miss , per_cache_miss[i]);

		len = strlen(line_buf);
		while( len != 0)
		{
			ret = write(mi_state->fd_measure, line_buf , len);
			if(ret < 0)
			{
				perror("Error: write file"); 
				goto error_test3;
			}
			len = len - ret;
		}
	}
	
	/* min, max, avg, var */
	ret = data_synthesis(mi_state , time_diff , per_cache_miss , name_test);
	if(ret < 0)
	{
		printf("Error: data synthesis \n");
		goto error_test3;
	}

	free(per_cache_miss);
	free(time_diff);
	return 0;

error_test3:	
	free(per_cache_miss);
	
error_test2:
	free(time_diff);

error_test1:
	return ret;
}

/**
 * start all tests
 * @mi_state: main descriptor for test 
 * return: zero on success or negative value for error
*/ 
int start_measure(struct mibench_state* mi_state)
{
	char s[N_ARG][SIZE_ARG];
	int ret;
	
	ret = write_first_row(mi_state->fd_measure , TYPE_FILE_MEASURE);
	if( ret < 0)
		return ret;
	
	ret = write_first_row(mi_state->fd_synthesis , TYPE_FILE_SYNTHESIS);	
	if(ret < 0)
		return ret;
	
	/* start susan small */
	memset(s[0] , '\0' , SIZE_ARG);
	memset(s[1] , '\0' , SIZE_ARG);
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);	
	strcpy(s[0] , "./susan");
	strcpy(s[1] , "input_small.pgm" );
	strcpy(s[2] , "output_small.smoothing.pgm" );
	strcpy(s[3] , "-s" );
	ret = start_test_susan(s , mi_state , "susan_small_smoothing");
	if(ret != 0)
		return ret;
	
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);
	strcpy(s[2] , "output_small.edges.pgm" );
	strcpy(s[3] , "-e" );	
	ret = start_test_susan(s , mi_state , "susan_small_edge");
	if(ret != 0)
		return ret;
		
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);	
	strcpy(s[2] , "output_small.corners.pgm" );
	strcpy(s[3] , "-c" );
	ret = start_test_susan(s , mi_state , "susan_small_corners");
	if(ret != 0)
		return ret;
	/* end susan small */

	/* start susan large */
	memset(s[0] , '\0' , SIZE_ARG);
	memset(s[1] , '\0' , SIZE_ARG);
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);	
	strcpy(s[0] , "./susan");
	strcpy(s[1] , "input_large.pgm" );
	strcpy(s[2] , "output_large.smoothing.pgm" );
	strcpy(s[3] , "-s" );
	ret = start_test_susan(s , mi_state , "susan_large_smoothing");
	if(ret != 0)
		return ret;

	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);
	strcpy(s[2] , "output_large.edges.pgm" );
	strcpy(s[3] , "-e" );
	ret = start_test_susan(s , mi_state , "susan_large_edges");
	if(ret != 0)
		return ret;
	
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);	
	strcpy(s[2] , "output_large.corners.pgm" );
	strcpy(s[3] , "-c" );
	ret = start_test_susan(s , mi_state , "susan_large_corners");
	if(ret != 0)
		return ret;
	/* end susan large */

	/* start susan huge */
	memset(s[0] , '\0' , SIZE_ARG);
	memset(s[1] , '\0' , SIZE_ARG);
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);	
	strcpy(s[0] , "./susan");
	strcpy(s[1] , "input_huge.pgm" );
	strcpy(s[2] , "output_huge.smoothing.pgm" );
	strcpy(s[3] , "-s" );
	ret = start_test_susan(s , mi_state , "susan_huge_smoothing");
	if(ret != 0)
		return ret;

	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);
	strcpy(s[2] , "output_huge.edges.pgm" );
	strcpy(s[3] , "-e" );
	ret = start_test_susan(s , mi_state , "susan_huge_edges");
	if(ret != 0)
		return ret;
	
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);	
	strcpy(s[2] , "output_huge.corners.pgm" );
	strcpy(s[3] , "-c" );
	ret = start_test_susan(s , mi_state , "susan_huge_corners");
	if(ret != 0)
		return ret;
	/* end susan huge */

	/* start qsort small */
	memset(s[0] , '\0' , SIZE_ARG);
	memset(s[1] , '\0' , SIZE_ARG);
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);
	strcpy(s[0] , "./qsort_small");
	strcpy(s[1] , "input_small.dat" );
	ret = start_test_qsort_s(s , mi_state , "qsort_small");
	if(ret != 0)
		return ret;
	/* end qsort small */

	/* start qsort large */
	memset(s[0] , '\0' , SIZE_ARG);
	memset(s[1] , '\0' , SIZE_ARG);
	memset(s[2] , '\0' , SIZE_ARG);
	memset(s[3] , '\0' , SIZE_ARG);
	strcpy(s[0] , "./qsort_large");
	strcpy(s[1] , "input_large.dat" );
	ret = start_test_qsort_l(s , mi_state , "qsort_large");
	if(ret != 0)
		return ret;
	/* end qsort large */;
	
	return 0;
}

/**
 * start all tests in thread mode
 * @mi_state: main descriptor for test 
*/ 
void *start_measure_th(void *data)
{
	struct mibench_state* mi_state;
	cpu_set_t cpuset;
	int ret;
	
	mi_state = (struct mibench_state*) data ;
	
	CPU_ZERO(&cpuset);
	CPU_SET( ID_CPU, &cpuset);	
	
	ret = pthread_setaffinity_np(mi_state->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_setaffinity_np() \n");
    goto err_th1;
  }
  /* Check the actual affinity mask assigned to the thread */
  ret = pthread_getaffinity_np(mi_state->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_getaffinity_np()");
    goto err_th1;
  }
	
	ret = start_measure(mi_state);
  if (ret != 0)
    goto err_th1;
	
	mi_state->state_thread = TH_STATE_OK;
  pthread_exit(NULL);

err_th1:  
	mi_state->state_thread = TH_STATE_FAIL;
  pthread_exit(NULL);
}

/**
 * init thread for test
 * @mi_state: main descriptor for test 
 * return: zero on success or negative value for error
*/ 
int init_test(struct mibench_state* mi_state)
{
	pthread_attr_t attr;
	int ret;
	
	ret = pthread_attr_init(&attr);
	if(ret)
	{
			printf("Error: pthread_attr_init() \n" ); 
			goto err_start1;
	}
	
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if(ret)
	{
			printf("Error: pthread_attr_setdetachstate() \n"); 
			goto err_start2;
	}
	
	mi_state->state_thread = TH_STATE_INIT;

	ret = pthread_create(&(mi_state->id_thread), &attr, (start_measure_th), (void *) mi_state);

	if(ret)
	{
		printf("Error: pthread_create() \n"); 
		goto err_start2;
	}
	
	ret = pthread_join(mi_state->id_thread, NULL);

	if(ret || mi_state->state_thread != TH_STATE_OK)
	{
		printf("Error: pthread_join() \n");
		
		if(ret >= 0)
			ret = -1;
	}
	
	pthread_attr_destroy(&attr);	
	return 0;
	
err_start2:
	pthread_attr_destroy(&attr);

err_start1:
	return ret;
}
