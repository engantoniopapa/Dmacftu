#include "daemon.h"
#include "daemon_operation.h"

/* delete the vector of memory region */
void del_region(struct region_mem *region)
{	
	if(region)
		free(region);
}

/**
 * init the vector of memory region 
 * @n_cpu: size of memory region vector
 * return: address of the vector on success or NULL for error
 */
struct region_mem * init_region(int n_cpu)
{
	struct region_mem *ptr;
	ptr = (struct region_mem *) malloc ( sizeof(struct region_mem) * n_cpu );
	
	if(!ptr)
	{
		perror("error init memory region");
		return NULL;
	}
	
	ptr->count = 0;
		
	return ptr;
}

/**
 * init daemon_trace
 * @daemon_ptr: struct daemon_trace
 * @n_cpu: number of cpu
 * @n_read: to calculate the max size of sparse file
 * @dev_name: device name
 */
void init_daemon(struct daemon_trace *daemon_ptr ,int n_cpu , int n_read , char *dev_name)
{		
	daemon_ptr->fd_sp = -1;
	daemon_ptr->fd_fl = -1;
	daemon_ptr->size_max = MAX_SIZE_SP(n_read , n_cpu);
	daemon_ptr->offset_map = 0;
	
	memset(daemon_ptr->path_file_sp , '\0' , SIZE_PATH );
	memset(daemon_ptr->path_file_fl , '\0' , SIZE_PATH );

	sprintf(daemon_ptr->path_file_sp , "../risultati/%s.temp" , dev_name);
	sprintf(daemon_ptr->path_file_fl , "../risultati/%s.dat" , dev_name );
	
}

/**
 * Discard old measures on the kernel buffer
 * @fd_dev: device file descriptor
 * @region: memory region for store the old measures
 * @size_r: size of @region (in byte)
 * return: zero on success or negative value for error
 */
int clean_kernel_buffer(int fd_dev , struct region_mem *region , size_t size_r)
{
	int i ;
	int n_read_clean = ((int) (KERNEL_UNIT_BUFFER / N_UNIT)) +1;

	for( i = 0; i < n_read_clean ; ++i)
	{
		memset(region , '0' , size_r);
			
		if( read(fd_dev, region, N_UNIT) != size_r)
		{
			perror("error dev file read()"); 
			return -EFAULT ;
		}
	}
	
	return 0;
}

/**
 * calculates the time difference (in ns)
 * @start: start time (in ns)
 * @end: end time (in ns)
 * return: the time difference (in ns) or zero if @start > @end
 */
unsigned long long diff_time(unsigned long long start , unsigned long long end)
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
 * read the measures to device file and write the measures in the sparse file
 * @ptr_daemon: struct daemon_trace
 * @n_cpu: number of cpu
 * @region: memory region for read to device file
 * @fd_dev: device file descriptor
 * return: zero on success or negative value for error
 */
int save_measure(struct daemon_trace *ptr_daemon , int n_cpu , 
		struct region_mem *region, int fd_dev)
{
	int i , j;
	struct unit_buff_user *ptr_unit = NULL;
	unsigned long long time_op;
	long page_size = sysconf(_SC_PAGESIZE); //la size di una page nel mio sistema
	off_t offset_to_page;
	size_t size_map;
	char * map = NULL ;
	
	memset(region , '0' , (sizeof(struct region_mem) * n_cpu) );
		
	/*read() to device file*/ 	
	if( read(fd_dev, region, N_UNIT) !=  (sizeof(struct region_mem) * n_cpu))
	{
		perror("error dev file read()"); 
		return -EFAULT ;
	}

	int count_region = 0;
	for( i = 0; i < n_cpu ; ++i)
	{
	  count_region = count_region + region[i].count;
	}
	
	if(count_region > 0)
	{  
		/* calculate offset as a multiple page size */
		offset_to_page = ptr_daemon->offset_map - (ptr_daemon->offset_map % page_size) ; 

		/* the size of mmap is: offset of last page + size of new measures */
		size_map = (ptr_daemon->offset_map % page_size ) + (N_CHAR_LINE * count_region);
	
		/* mapping of sparse file region */
		map = (char*) mmap(NULL , size_map , PROT_READ | PROT_WRITE, MAP_SHARED, ptr_daemon->fd_sp, offset_to_page );
		if( map == MAP_FAILED ) 
		{
			perror("Error mmapping the file");
			return -1;
		}
		
		char *type_op;
	
		for( i = 0; i < n_cpu ; ++i)
		{
					
			for( j = 0  ; j < region[i].count; ++j)
			{
				ptr_unit = &(region[i].buffer[j]);
				time_op = diff_time(ptr_unit->clock_cpu_start , ptr_unit->clock_cpu_end);
				
				switch(ptr_unit->type_op)
				{
					case OP_CPU:
							 type_op = "CPU";
							 break;

					case OP_DMA:
							 type_op = "DMA";
							 break;

					 default:
							 type_op ="NO DEF";
							 break;
				}
	
					/* write measure in memory mapped */
				sprintf(&(map[strlen(map)]),"%d    %13llu%17lu%10d%21llu%21llu%15llu%24p%13s\n", ptr_unit->index_cpu_event, 
								ptr_unit->count_event ,	ptr_unit->size , ptr_unit->index_cpu , ptr_unit->clock_cpu_start , 
								ptr_unit->clock_cpu_end , time_op ,ptr_unit->addr , type_op);
	
			}
		}
	
		/* update offset for mapping */
		ptr_daemon->offset_map = offset_to_page + strlen(map);	
		
		/* memory unmapping */
		if (munmap(map, size_map) == -1) 
		{
			perror("Error un-mmapping the file");
			return -EINVAL;
		}	
	}
	
	/* lseek to device file for new read() */
	if( lseek(fd_dev , 0 , SEEK_SET) < 0)
	{
		perror("error lseek dev file");
		return -EINVAL;
	}
		
	return 0;
}

/**
 * save the measures in the regular file 
 * @daemon_ptr: struct daemon_trace
 * @n_cpu: n_cpu
 * @n_read: number of read() to device file
 * @delay: dely (in msec) between two read() to device file
 * @path_dev: path name of device file descriptor
 * return: zero on success or negative value for error
*/
int	record_measures(struct daemon_trace *daemon_ptr , int n_cpu , 
		unsigned long n_read , unsigned long delay , char *path_dev)
{
	int ret;
	int fd_dev;
	int flag = O_RDWR | O_CREAT | O_TRUNC ;
	struct region_mem *region = NULL; // regione di memoria da passare alla read()
	
	ret = daemon_create_sparse_file(daemon_ptr , flag , (mode_t)0666);
	if(ret < 0)
		goto err_rec1;
	
	ret = sparse_first_row(daemon_ptr);
	if(ret < 0)
		goto err_rec2;

	region = init_region(n_cpu);
	if(!region)
		goto err_rec2;
	
	fd_dev = open(path_dev, O_RDWR);
	if(fd_dev < 0)
	{
		perror("error open dev file");
		goto err_rec3;
	}
	
	ret = clean_kernel_buffer(fd_dev , region , (size_t) (sizeof(struct region_mem) * n_cpu) );
	if(ret < 0)
		goto err_rec4;

	int i;
	for( i = 0 ; i < n_read ; ++i)
	{
		ret = save_measure(daemon_ptr ,n_cpu , region , fd_dev);
		if(ret < 0)
		  goto err_rec4;
			
		usleep(delay); // delay between read() 
	}
	
	daemon_ptr->fd_fl = open(daemon_ptr->path_file_fl ,flag, (mode_t)0666);
	if(daemon_ptr->fd_fl < 0)
	{
		printf("error to create regular file\n");
		ret = -EINVAL;
		goto err_rec4;
	}	
	
	ret= lseek(daemon_ptr->fd_sp, 0 , SEEK_SET);
	if(ret < 0)
	{
		printf("error to lseek sparse file\n");
		goto err_rec4;
	}	

	
	ret = copy_normal_file(daemon_ptr);
	if(ret < 0)
	{
		printf("error to copy file sparse in regular file\n");
		goto err_rec5;
	}
	
	close(daemon_ptr->fd_fl);
	close(fd_dev);
	del_region(region);
	close(daemon_ptr->fd_sp);
	
	ret = remove(daemon_ptr->path_file_sp);
	if(ret < 0)
		printf("remove file sparse error \n");
	
	return 0;

err_rec5:
	close(daemon_ptr->fd_fl);

err_rec4:
	close(fd_dev);

err_rec3:
	del_region(region);
	
err_rec2:
	close(daemon_ptr->fd_sp);
	
err_rec1:
	return ret;
}
