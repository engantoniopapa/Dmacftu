#include "daemon.h"

/** thread function to measure copy_to/from_user
 * @data: object of struct thread_benchmark
*/
void *start_thread_from_to(void *data)
{
	struct thread_benchmark *th_data;
	int ret;
	int i;
	char  *tmp_buf;
	cpu_set_t cpuset;

	th_data = (struct thread_benchmark *) data;	
	
	for( i = 0 ; i < th_data->size_region ;++i)
			th_data->region[i] = (char) (48+(i%10)) ;
			
	CPU_ZERO(&cpuset);
	CPU_SET( ID_CPU, &cpuset);	
	
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

	/* end setup device file */
	pthread_mutex_lock(&queue_lock);

		++count_queue;
		if(count_queue == N_CHAN)
			pthread_cond_broadcast(&queue_full);
		else
			pthread_cond_wait(&queue_full , &queue_lock);
	
	pthread_mutex_unlock(&queue_lock);
	
	/* start operation */
	while(!end_pthread)
	{	
		i =  th_data->counter_op % ((int) (th_data->size_region / th_data->size_message));
		tmp_buf = th_data->region + (i * th_data->size_message);
		ret = read(th_data->fd_dev ,tmp_buf, th_data->size_message);
		if (ret < 0)
		{
			perror("error read dev-file"); 
			goto err_th2;
		}
		
		if( ret != th_data->size_message)
			printf("read-dev: size = %lu , ret = %d \n" , th_data->size_message , ret);
			
		++(th_data->counter_op);	
		
		i =  th_data->counter_op % ((int) (th_data->size_region / th_data->size_message));
		tmp_buf = th_data->region + (i * th_data->size_message);
		ret = write(th_data->fd_dev, tmp_buf , th_data->size_message);
		if (ret < 0)
		{
			perror("error write dev-file"); 
			goto err_th2;
		}
		
		if( ret != th_data->size_message)
			printf("write-dev: size = %lu , ret = %d \n" , th_data->size_message , ret);
			
		++(th_data->counter_op);	
	}
	/* end operation */

	close(th_data->fd_dev);
	th_data->state_thread = TH_STATE_OK;
  pthread_exit(NULL);
	
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
	int ret;
	int i;
	char  *tmp_buf;
	cpu_set_t cpuset;
	
	th_data = (struct thread_benchmark *) data;	
	
	for( i = 0 ; i < th_data->size_region ;++i)
			th_data->region[i] = (char) (48+(i%10)) ;
			
	CPU_ZERO(&cpuset);
	CPU_SET( ID_CPU, &cpuset);	
	
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

	/* end setup device file */
	pthread_mutex_lock(&queue_lock);

		++count_queue;
		if(count_queue == N_CHAN)
			pthread_cond_broadcast(&queue_full);
		else
			pthread_cond_wait(&queue_full , &queue_lock);
	
	pthread_mutex_unlock(&queue_lock);
	
	/* start operation */
	while(!end_pthread)
	{			
		i =  th_data->counter_op % ((int) (th_data->size_region / th_data->size_message));
		tmp_buf = th_data->region + (i * th_data->size_message);
		ret = write(th_data->fd_dev, tmp_buf , th_data->size_message);
		if (ret < 0)
		{
			perror("error write dev-file"); 
			goto err_th2;
		}
		
		if( ret != th_data->size_message)
			printf("write-dev: size = %lu , ret = %d \n" , th_data->size_message , ret);
		
		++(th_data->counter_op);	
	}
	/* end operation */

	close(th_data->fd_dev);
	th_data->state_thread = TH_STATE_OK;
  pthread_exit(NULL);
	
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
	int ret;
	int i;
	char  *tmp_buf;
	cpu_set_t cpuset;
	
	th_data = (struct thread_benchmark *) data;	
	
	for( i = 0 ; i < th_data->size_region ;++i)
			th_data->region[i] = (char) (48+(i%10)) ;
			
	CPU_ZERO(&cpuset);
	CPU_SET( ID_CPU, &cpuset);	
	
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

	/* end setup device file */
	pthread_mutex_lock(&queue_lock);

		++count_queue;
		if(count_queue == N_CHAN)
			pthread_cond_broadcast(&queue_full);
		else
			pthread_cond_wait(&queue_full , &queue_lock);
	
	pthread_mutex_unlock(&queue_lock);
	
	/* start operation */
	while(!end_pthread)
	{	
		i =  th_data->counter_op % ((int) (th_data->size_region / th_data->size_message));
		tmp_buf = th_data->region + (i * th_data->size_message);
		ret = read(th_data->fd_dev ,tmp_buf, th_data->size_message);
		if (ret < 0)
		{
			perror("error read dev-file"); 
			goto err_th2;
		}
		
		if( ret != th_data->size_message)
			printf("read-dev: size = %lu , ret = %d \n" , th_data->size_message , ret);
			
		++(th_data->counter_op);		
	}
	/* end operation */

	close(th_data->fd_dev);
	th_data->state_thread = TH_STATE_OK;
  pthread_exit(NULL);
	
err_th2:
	close(th_data->fd_dev);

err_th1:  
	th_data->state_thread = TH_STATE_FAIL;
  pthread_exit(NULL);
}

/** start the measures for different size messages
 * @th_data: main descriptor for thread state  
 * @size: size messages
 * @(*func_copy): thread function to measure
 * return: zero on success or negative value for error
*/
int start_micro_benchmark(struct thread_benchmark *th_data , unsigned long size,
		void *(*func_copy)(void *data) )
{
	char *region_mem;
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

  ret = pthread_mutex_init(&queue_lock , NULL) ;
	if(ret)
	{
		printf("Error: pthread_mutex_init() \n"); 
		goto err_start2;
	}
		
	ret = pthread_cond_init(&queue_full , NULL);
	if(ret)
	{
		printf("Error: pthread_cond_init() \n"); 
		goto err_start3;
	}
	
	region_mem = (char *) malloc( (size_t) (SIZE_REG_MEM * N_CHAN));
	
	if(!region_mem)
	{
		printf("Error: malloc() on region_mem \n"); 
		ret = -ENOMEM;
		goto err_start4;
	}
	
	int j;
	count_queue = 0;
	end_pthread = 0;

	for( j = 0; j < N_CHAN ; ++j)
	{
		th_data[j].size_region = SIZE_REG_MEM;
		th_data[j].size_message = size;
		th_data[j].state_thread = TH_STATE_INIT;
		th_data[j].id_chan = j;
		th_data[j].counter_op = 0 ;
		th_data[j].region = &region_mem[SIZE_REG_MEM*j];

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
			goto err_start5;
		} 
	}
	
	printf("\nPress a key to end....");
  getchar(); 
 	end_pthread = 1;
	printf("\n");

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
			
			goto err_start5;
		}
	}	
	
	free(region_mem);	
	pthread_cond_destroy(&queue_full);
	pthread_mutex_destroy(&queue_lock);	
	pthread_attr_destroy(&attr);
			
	return 0;

err_start5:
	free(region_mem);
	
err_start4:
	pthread_cond_destroy(&queue_full);

err_start3:
	pthread_mutex_destroy(&queue_lock);

err_start2:
	pthread_attr_destroy(&attr);

err_start1:
	return ret;
}
