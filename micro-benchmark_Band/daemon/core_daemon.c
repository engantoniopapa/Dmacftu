#include "daemon.h"

/**
 * calculates the time difference (in ns)
 * @start: start time (in ns)
 * @end: end time (in ns)
 * return: the time difference (in ns) or zero if @start > @end
 */
static unsigned long long diff_time(unsigned long long start , unsigned long long end)
{
	unsigned long long time_op;

	if(start > end)
	{
		time_op = 0;
	}
	else
	{
		time_op = end - start;
	}
	
	return time_op;
}

/** 
 * write first row in a file, the # is used for gnuplot 
 * @fd: file descriptor
 * return: zero on success or negative value for error
 */	
int static write_first_row(int fd)
{	
	char line_buf[N_CHAR_LINE];
	int ret;
	int len;
	
	memset(line_buf, '\0' , N_CHAR_LINE);

	strncpy(line_buf , "# id_chan        Size_Msg    n_op        StartTime___(ns)          EndTime___(ns)"
	"         DiffTime(ns)    Bandwidth(MB/sec)\n#\n", N_CHAR_LINE );
	
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

/** thread function to measure copy_to/from_user
 * @data: object of struct thread_benchmark
*/
void *start_thread_from_to(void *data)
{
	struct thread_benchmark *th_data;
	query_arg_t q_dev;
	int ret;
	int i;
	struct timespec tspec_start , tspec_end;
	cpu_set_t cpuset;

	
	th_data = (struct thread_benchmark *) data;	
	
	for( i = 0 ; i < th_data->size ;++i)
			th_data->buf[i] = (char) (48+(i%10)) ;
			
	CPU_ZERO(&cpuset);
	CPU_SET( (th_data->id_chan % N_CPU ), &cpuset);	
	
	ret = pthread_setaffinity_np(th_data->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_setaffinity_np() \n");
    goto err_th1;
  }
  /* Check the actual affinity mask assigned to the thread */
  ret = pthread_getaffinity_np(th_data->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_getaffinity_np()");
    goto err_th1;
  }
	
	/* setup device file */ 
	th_data->fd_dev = open(th_data->path_dev, O_RDWR);
	if(th_data->fd_dev < 0)
	{
		perror("error open dev file");
		goto err_th1;
	}
	
	q_dev.write = 1;
  q_dev.buf_user = th_data->buf;  
  q_dev.len = th_data->size;
	
	if( ioctl(th_data->fd_dev, MB_LOAD, &q_dev) )
	{
		printf("Error: ioctl() , MB_LOAD \n"); 
		goto err_th2;
	}
	/* end setup device file */
	pthread_mutex_lock(&queue_lock);

		++count_queue;
		if(count_queue == N_CHAN)
			pthread_cond_broadcast(&queue_full);
		else
			pthread_cond_wait(&queue_full , &queue_lock);
	
	pthread_mutex_unlock(&queue_lock);
		
	/* start measure */
	asm volatile("mfence":::"memory");		
		if (clock_gettime( CLOCK_REALTIME , &tspec_start ) != 0)
		{
			perror ("clock_gettime():");
			goto err_th3;
		}
	asm volatile("mfence":::"memory");		
	
	/* start operation */
	for( i = 0 ; i < th_data->n_op; ++i)
	{	
		ret = read(th_data->fd_dev ,th_data->buf, th_data->size);
		if (ret < 0)
		{
			perror("error read dev-file"); 
			goto err_th3;
		}
		if( ret != th_data->size)
			printf("read-dev: size = %lu , ret = %d \n" , th_data->size , ret);
		
		ret = write(th_data->fd_dev, th_data->buf, th_data->size);
		if (ret < 0)
		{
			perror("error write dev-file"); 
			goto err_th3;
		}
		if( ret != th_data->size)
			printf("write-dev: size = %lu , ret = %d \n" , th_data->size , ret);
	}
	/* end operation */

	asm volatile("mfence":::"memory");		
		if (clock_gettime(CLOCK_REALTIME, &tspec_end ) != 0)
		{
			perror ("clock_gettime():");
			goto err_th3;
		}
	asm volatile("mfence":::"memory");		
	/* end measure */

	th_data->time_start = tspec_start.tv_sec;
	th_data->time_start = (th_data->time_start * SEC_TO_NS) + tspec_start.tv_nsec;
	
	th_data->time_end = tspec_end.tv_sec;
	th_data->time_end = (th_data->time_end * SEC_TO_NS) + tspec_end.tv_nsec;

	if(ioctl(th_data->fd_dev, MB_UNLOAD, &q_dev))
	{
		printf("Error: ioctl() , MB_UNLOAD \n"); 
		goto err_th2;
	}

	close(th_data->fd_dev);
	th_data->state_thread = TH_STATE_OK;
  pthread_exit(NULL);

err_th3:
	if(ioctl(th_data->fd_dev, MB_UNLOAD, &q_dev))
		printf("Error: ioctl() , MB_UNLOAD \n"); 
	
err_th2:
	close(th_data->fd_dev);

err_th1:  
	th_data->state_thread = TH_STATE_FAIL;
  pthread_exit(NULL);
}

/** thread function to measure copy_from_user
 * @data: object of struct thread_benchmark
*/
void *start_thread_from(void *data)
{
	struct thread_benchmark *th_data;
	query_arg_t q_dev;
	int ret;
	int i;
	struct timespec tspec_start , tspec_end;
	cpu_set_t cpuset;

	
	th_data = (struct thread_benchmark *) data;	
	
	for( i = 0 ; i < th_data->size ;++i)
			th_data->buf[i] = (char) (48+(i%10)) ;
			
	CPU_ZERO(&cpuset);
	CPU_SET( (th_data->id_chan % N_CPU ), &cpuset);	
	
	ret = pthread_setaffinity_np(th_data->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_setaffinity_np() \n");
    goto err_th1;
  }
  /* Check the actual affinity mask assigned to the thread */
  ret = pthread_getaffinity_np(th_data->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_getaffinity_np()");
    goto err_th1;
  }
	
	/* setup device file */ 
	th_data->fd_dev = open(th_data->path_dev, O_RDWR);
	if(th_data->fd_dev < 0)
	{
		perror("error open dev file");
		goto err_th1;
	}
	
	q_dev.write = 1;
  q_dev.buf_user = th_data->buf;  
  q_dev.len = th_data->size;
	
	if( ioctl(th_data->fd_dev, MB_LOAD, &q_dev) )
	{
		printf("Error: ioctl() , MB_LOAD \n"); 
		goto err_th2;
	}
	/* end setup device file */
	pthread_mutex_lock(&queue_lock);

		++count_queue;
		if(count_queue == N_CHAN)
			pthread_cond_broadcast(&queue_full);
		else
			pthread_cond_wait(&queue_full , &queue_lock);
	
	pthread_mutex_unlock(&queue_lock);
		
	/* start measure */
	
	asm volatile("mfence":::"memory");		
		if (clock_gettime( CLOCK_REALTIME , &tspec_start ) != 0)
		{
			perror ("clock_gettime():");
			goto err_th3;
		}
	asm volatile("mfence":::"memory");		

	/* start operation */
	for( i = 0 ; i < th_data->n_op; ++i)
	{	
		ret = write(th_data->fd_dev, th_data->buf, th_data->size);
		if (ret < 0)
		{
			perror("error write dev-file"); 
			goto err_th3;
		}
		if( ret != th_data->size)
			printf("write-dev: size = %lu , ret = %d \n" , th_data->size , ret);
	}
	/* end operation */

  asm volatile("mfence":::"memory");		
		if (clock_gettime(CLOCK_REALTIME, &tspec_end ) != 0)
		{
			perror ("clock_gettime():");
			goto err_th3;
		}
	asm volatile("mfence":::"memory");		

	/* end measure */

	th_data->time_start = tspec_start.tv_sec;
	th_data->time_start = (th_data->time_start * SEC_TO_NS) + tspec_start.tv_nsec;
	
	th_data->time_end = tspec_end.tv_sec;
	th_data->time_end = (th_data->time_end * SEC_TO_NS) + tspec_end.tv_nsec;

	if(ioctl(th_data->fd_dev, MB_UNLOAD, &q_dev))
	{
		printf("Error: ioctl() , MB_UNLOAD \n"); 
		goto err_th2;
	}

	close(th_data->fd_dev);
	th_data->state_thread = TH_STATE_OK;
  pthread_exit(NULL);

err_th3:
	if(ioctl(th_data->fd_dev, MB_UNLOAD, &q_dev))
		printf("Error: ioctl() , MB_UNLOAD \n"); 
	
err_th2:
	close(th_data->fd_dev);

err_th1:  
	th_data->state_thread = TH_STATE_FAIL;
  pthread_exit(NULL);
}

/** thread function to measure copy_to_user
 * @data: object of struct thread_benchmark
*/
void *start_thread_to(void *data)
{
	struct thread_benchmark *th_data;
	query_arg_t q_dev;
	int ret;
	int i;
	struct timespec tspec_start , tspec_end;
	cpu_set_t cpuset;

	
	th_data = (struct thread_benchmark *) data;	
	
	for( i = 0 ; i < th_data->size ;++i)
			th_data->buf[i] = (char) (48+(i%10)) ;
			
	CPU_ZERO(&cpuset);
	CPU_SET( (th_data->id_chan % N_CPU ), &cpuset);	
	
	ret = pthread_setaffinity_np(th_data->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_setaffinity_np() \n");
    goto err_th1;
  }
  /* Check the actual affinity mask assigned to the thread */
  ret = pthread_getaffinity_np(th_data->id_thread, sizeof(cpu_set_t), &cpuset);
  if (ret != 0)
  {
    printf("Error: pthread_getaffinity_np()");
    goto err_th1;
  }
	
	/* setup device file */ 
	th_data->fd_dev = open(th_data->path_dev, O_RDWR);
	if(th_data->fd_dev < 0)
	{
		perror("error open dev file");
		goto err_th1;
	}
	
	q_dev.write = 1;
  q_dev.buf_user = th_data->buf;  
  q_dev.len = th_data->size;
	
	if( ioctl(th_data->fd_dev, MB_LOAD, &q_dev) )
	{
		printf("Error: ioctl() , MB_LOAD \n"); 
		goto err_th2;
	}
	/* end setup device file */
	pthread_mutex_lock(&queue_lock);

		++count_queue;
		if(count_queue == N_CHAN)
			pthread_cond_broadcast(&queue_full);
		else
			pthread_cond_wait(&queue_full , &queue_lock);
	
	pthread_mutex_unlock(&queue_lock);
		
	/* start measure */
	asm volatile("mfence":::"memory");		
		if (clock_gettime( CLOCK_REALTIME , &tspec_start ) != 0)
		{
			perror ("clock_gettime():");
			goto err_th3;
		}
	asm volatile("mfence":::"memory");		
	
	/* start operation */
	for( i = 0 ; i < th_data->n_op; ++i)
	{	
		ret = read(th_data->fd_dev ,th_data->buf, th_data->size);
		if (ret < 0)
		{
			perror("error read dev-file"); 
			goto err_th3;
		}
		if( ret != th_data->size)
			printf("read-dev: size = %lu , ret = %d \n" , th_data->size , ret);
	}
	/* end operation */
	
	asm volatile("mfence":::"memory");		
		if (clock_gettime(CLOCK_REALTIME, &tspec_end ) != 0)
		{
			perror ("clock_gettime():");
			goto err_th3;
		}
	asm volatile("mfence":::"memory");		
	/* end measure */

	th_data->time_start = tspec_start.tv_sec;
	th_data->time_start = (th_data->time_start * SEC_TO_NS) + tspec_start.tv_nsec;
	
	th_data->time_end = tspec_end.tv_sec;
	th_data->time_end = (th_data->time_end * SEC_TO_NS) + tspec_end.tv_nsec;

	if(ioctl(th_data->fd_dev, MB_UNLOAD, &q_dev))
	{
		printf("Error: ioctl() , MB_UNLOAD \n"); 
		goto err_th2;
	}

	close(th_data->fd_dev);
	th_data->state_thread = TH_STATE_OK;
  pthread_exit(NULL);

err_th3:
	if(ioctl(th_data->fd_dev, MB_UNLOAD, &q_dev))
		printf("Error: ioctl() , MB_UNLOAD \n"); 
	
err_th2:
	close(th_data->fd_dev);

err_th1:  
	th_data->state_thread = TH_STATE_FAIL;
  pthread_exit(NULL);
}

/** start the measures for different size messages
 * @th_data: main descriptor for thread state  
 * @max_size: upper bound of size messages
 * @(*func_copy): thread function to measure
 * @path_result: path file measure result
 * @type: type operation
 * return: zero on success or negative value for error
*/
int start_micro_benchmark(struct thread_benchmark *th_data , unsigned long max_size,
		void *(*func_copy)(void *data) , char *path_result, short int type)
{
	int fd;
	int flag = O_RDWR | O_CREAT | O_TRUNC ;
	char line_buf[N_CHAR_LINE];
	int len;
	int ret;
	unsigned long long dif_time;
	double bandwidth;
	double tot_band; /* bandwith of all chan */
	char *region_mem;
	pthread_attr_t attr;

	fd = open(path_result ,flag, (mode_t)0666);
	if(fd < 0)
	{
		perror("open measures file error");
		ret = fd;
		goto err_start1;
	}
	
	ret = write_first_row(fd);
	if( ret < 0)
		goto err_start2;
	
	ret = pthread_attr_init(&attr);
	if(ret)
	{
			printf("Error: pthread_attr_init() \n" ); 
			goto err_start2;
	}
	
	ret =	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if(ret)
	{
			printf("Error: pthread_attr_setdetachstate() \n"); 
			goto err_start3;
	}

	ret = pthread_mutex_init(&queue_lock , NULL) ;
	if(ret)
	{
		printf("Error: pthread_mutex_init() \n"); 
		goto err_start3;
	}
	
	ret = pthread_cond_init(&queue_full , NULL);
	if(ret)
	{
		printf("Error: pthread_cond_init() \n"); 
		goto err_start4;
	}
	
	region_mem = (char *) malloc( (size_t) (max_size * N_CHAN));
	
	if(!region_mem)
	{
		printf("Error: malloc() on region_mem \n"); 
		ret = -ENOMEM;
		goto err_start5;
	}
	
	unsigned long i;
	int j;
	
/* for cache */	
	count_queue = 0;
	for( j = 0; j < N_CHAN ; ++j)
	{
		th_data[j].size = _1KB_TO_BYTE;
		th_data[j].state_thread = TH_STATE_INIT;
		th_data[j].id_chan = j;
		th_data[j].buf = &region_mem[_1KB_TO_BYTE*j];

		ret = pthread_create(&(th_data[j].id_thread), &attr, (*func_copy), (void *) &th_data[j]);
		
		if(ret)
		{
			printf("Error: pthread_create() \n"); 
			--j;
			for( ; j <= 0 ; --j)
			{
				if(pthread_cancel(th_data[j].id_thread))
					printf("Error: pthread_cancel() \n"); 
			}
			goto err_start6;
		} 
	}
	usleep(10000);
	for( j = 0; j < N_CHAN ; ++j)
	{
		ret = pthread_join(th_data[j].id_thread, NULL);
		
		if(ret || th_data[j].state_thread != TH_STATE_OK)
		{
			printf("Error: pthread_join() \n"); 
			++j;
			for( ; j < N_CHAN  ; ++j)
				pthread_cancel(th_data[j].id_thread);
			
			if(ret >= 0)
				ret = -1;
				
			goto err_start6;
		}
	}	
/* end cache */
	
	for( i = _1KB_TO_BYTE ; i <= max_size ;)
	{		
		count_queue = 0;
		tot_band = 0;
		for( j = 0; j < N_CHAN ; ++j)
		{
			th_data[j].size = i;
			th_data[j].state_thread = TH_STATE_INIT;
			th_data[j].id_chan = j;
			th_data[j].buf = &region_mem[i*j];

			ret = pthread_create(&(th_data[j].id_thread), &attr, (*func_copy), (void *) &th_data[j]);
			
			if(ret)
			{
				printf("Error: pthread_create() \n"); 
				--j;
				for( ; j <= 0 ; --j)
				{
					if(pthread_cancel(th_data[j].id_thread))
						printf("Error: pthread_cancel() \n"); 
				}
				goto err_start6;
			} 
		}

		for( j = 0; j < N_CHAN ; ++j)
		{
			ret = pthread_join(th_data[j].id_thread, NULL);
			
			if(!ret && th_data[j].state_thread == TH_STATE_OK)
			{
				dif_time = diff_time(th_data[j].time_start , th_data[j].time_end);
				
				bandwidth = ((double) th_data[j].size ) / _1MB_TO_BYTE;
				bandwidth = bandwidth * th_data[j].n_op;
				bandwidth = bandwidth  /( ((double)dif_time )/ SEC_TO_NS );
				
				/* thread makes copy_from_user and copy_to_user */ 
				if(type == TYPE_MEASURE_FROM_TO)
					bandwidth = bandwidth *2;
				
				tot_band = tot_band +bandwidth;
				memset(line_buf, '\0' , N_CHAR_LINE);
				
				if( j != (N_CHAN -1) )	
				{
					sprintf(line_buf," %8d%16lu%8lu%24llu%24llu%21llu%21.6f\n", th_data[j].id_chan, th_data[j].size,
							  th_data[j].n_op, th_data[j].time_start, th_data[j].time_end, dif_time ,bandwidth);
				}
				else
				{			  
					sprintf(line_buf," %8d%16lu%8lu%24llu%24llu%21llu%21.6f -> total_band = %15.6f\n#\n", 
						th_data[j].id_chan, th_data[j].size, th_data[j].n_op, th_data[j].time_start, th_data[j].time_end, 
						dif_time , bandwidth , tot_band);
				}

				len = strlen(line_buf);
				while( len != 0)
				{
					ret = write(fd, line_buf , len);
					if(ret < 0)
					{
						perror("Error: write file"); 
						++j;
						for( ; j < N_CHAN  ; ++j)
							pthread_cancel(th_data[j].id_thread);
							
						goto err_start6;
					}
					len = len - ret;
				}
			}
			else
			{
				printf("Error: pthread_join() \n"); 
				++j;
				for( ; j < N_CHAN  ; ++j)
					pthread_cancel(th_data[j].id_thread);
				
				if(ret >= 0)
					ret = -1;
					
				goto err_start6;
			}
		}	
		i = i << 1;	
		usleep(10000);
	}
	
	free(region_mem);	
	pthread_cond_destroy(&queue_full);
	pthread_mutex_destroy(&queue_lock);	
	pthread_attr_destroy(&attr);
		
	close(fd);
	
	return 0;

err_start6:
	free(region_mem);
	
err_start5:
	pthread_cond_destroy(&queue_full);

err_start4:
	pthread_mutex_destroy(&queue_lock);

err_start3:
	pthread_attr_destroy(&attr);

err_start2:
	close(fd);
	
err_start1:
	return ret;
}
