#include "daemon.h"

extern int data_synthesis(int fd , unsigned int count , unsigned long long *time_op, unsigned long size);

/**
 * init the vector of memory region 
 * return: address of the vector on success or NULL for error
 */
struct region_mem * init_region(void)
{
	struct region_mem *ptr;
	ptr = (struct region_mem *) malloc ( sizeof(struct region_mem) );
	
	if(!ptr)
	{
		perror("error init memory region");
		return NULL;
	}
	
	ptr->count = 0;
		
	return ptr;
}

/**
 * init a struct daemon_benchmark
 * return: address of the struct on success or NULL for error
 */ 
struct daemon_benchmark *init_daemon_benchmark(void)
{
	struct daemon_benchmark *daemon_ptr;
	
	daemon_ptr = (struct daemon_benchmark *) malloc ( sizeof(struct daemon_benchmark ) );
	if(!daemon_ptr)
		return NULL;
	
	daemon_ptr->fd_fl = -1;
	daemon_ptr->fd_synth = -1;

	return daemon_ptr;	
}

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
 * @type: ID_MEASURE or ID_SYNTH
 * return: zero on success or negative value for error
 */	
int static write_first_row(int fd , short int type)
{	
	char line_buf[N_CHAR_LINE];
	int ret;
	int len;
	
	memset(line_buf, '\0' , N_CHAR_LINE);

	if(type == ID_MEASURE)
		strncpy(line_buf , "#index_cpu    Byte Copiati     StartTime___(ns)       EndTime___(ns)         DiffTime(ns)\n", N_CHAR_LINE );
	
	if(type == ID_SYNTH)
		strncpy(line_buf , "#    Byte Copiati           Min           Max             Average              Variance              Median\n", N_CHAR_LINE );
	
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
 * read the measures to device file and write the measures in the files
 * @fd_fl: result file descriptor 
 * @fd_synth: synthesis file descriptor 
 * @size: size of memory copy to test
 * @region: memory region store result from device file
 * @fd_dev: device file descriptor
 * return: zero on success or negative value for error
 */
int static save_measure(int  fd_fl , int fd_synth , unsigned long size , 
 struct region_mem *region, int fd_dev)
{
	int j;
	struct unit_buff_user *ptr_unit = NULL;
	unsigned long long *time_op;

	char *buf_write;
	char line_buf[N_CHAR_LINE];
	int len;
	int ret;

	buf_write = (char *) malloc (size);
	if(!buf_write)
	{
		perror("error memory"); 
		ret = -ENOMEM ;
		goto error_buf;
	}	
	
	memset(region , '0' , sizeof(struct region_mem)  );

	ret = write(fd_dev, buf_write , size);
	if( ret != size )
	{
		perror("error dev file read()"); 
		
		if(ret >= 0)
			ret = -EFAULT;
			
		goto error_op1;
	}
		
	ret = read(fd_dev, region, N_UNIT);
	if( ret != sizeof(struct region_mem))
	{
		perror("error dev file read()"); 
		
		if(ret >= 0)
			ret = -EFAULT;
			
		goto error_op1;
	}
	
	if(region->count < 0 )
	{
		printf("Error count measures"); 
		ret = -1;
		goto error_op1;
	}
	
	if(region->count == 0)
	{
		return 0;
	}
	
	time_op = (unsigned long long *) malloc (region->count * sizeof(unsigned long long));
	if(!time_op)
	{
		perror("error memory"); 
		ret = -ENOMEM ;
		goto error_op1;
	}	
		
	for( j = 0  ; j < region->count; ++j)
	{
		ptr_unit = &(region->buffer[j]);
		time_op[j] = diff_time(ptr_unit->clock_cpu_start , ptr_unit->clock_cpu_end);
		
		memset(line_buf, '\0' , N_CHAR_LINE);
		sprintf(line_buf," %9d%16lu%21llu%21llu%21llu\n", ptr_unit->index_cpu, 
						ptr_unit->size , ptr_unit->clock_cpu_start , ptr_unit->clock_cpu_end , time_op[j] );
		
		len = strlen(line_buf);
						
		while( len != 0)
		{
			ret = write(fd_fl, line_buf , len);
			if(ret < 0)
			{
				perror("error write file"); 
				goto error_op2;
			}
			len = len - ret;
		}
	}	
	
	/* lseek to device file for new measures */
	ret = lseek(fd_dev , 0 , SEEK_SET);
	if(ret < 0)
	{
		perror("error lseek dev file");
		goto error_op2;
	}
	
	/* min, max, avg, var */
	if( region->count > 0 )
	{
		ret = data_synthesis(fd_synth , region->count , time_op , size);
		if(ret < 0)
		{
			printf("Error: data synthesis \n");
			goto error_op2;
		}
	}
	
	free(time_op);
	free(buf_write);
	return 0;

error_op2:
	free(time_op);

error_op1:
	free(buf_write);
		
error_buf:
	return ret;
}

/**
 * save the measures in the regular file 
 * @daemon_b: for state of daemon
 * @max_size: max_size for test
 * @path_dev: path name of device file descriptor
 * return: zero on success or negative value for error
*/
int	record_measures(struct daemon_benchmark *daemon_b , unsigned long max_size , char *path_dev)
{
	int ret;
	int fd_dev;
	int flag = O_RDWR | O_CREAT | O_TRUNC ;
	struct region_mem *region = NULL; // regione di memoria da passare alla read()
	
	daemon_b->fd_fl = open(daemon_b->path_file_fl ,flag, (mode_t)0666);
	if(daemon_b->fd_fl < 0)
	{
			perror("open measures file error");
			ret = daemon_b->fd_fl;
			goto err_rec1;
	}
	
	daemon_b->fd_synth = open(daemon_b->path_file_synth ,flag, (mode_t)0666);
	if(daemon_b->fd_synth < 0)
	{
			perror("open synthesis file error");
			ret = daemon_b->fd_synth;
			goto err_rec2;
	}

	region = init_region();
	if(!region)
		goto err_rec3;
	
	fd_dev = open(path_dev, O_RDWR);
	if(fd_dev < 0)
	{
		perror("error open dev file");
		ret = fd_dev;
		goto err_rec4;
	}
	
	ret = write_first_row(daemon_b->fd_synth , ID_SYNTH);
	if(ret < 0)
		goto err_rec5;
			
	unsigned long i;
	for( i = 1 ; i <= max_size ;)
	{		
		ret = write_first_row(daemon_b->fd_fl , ID_MEASURE);
		if( ret < 0)
			goto err_rec5;
			
		ret = save_measure(daemon_b->fd_fl , daemon_b->fd_synth, i ,  region , fd_dev);
		if(ret < 0)
		  goto err_rec5;
				
		i = i << 1;	
	}
	
	close(fd_dev);
	free(region);
	close(daemon_b->fd_synth);
	close(daemon_b->fd_fl);
	daemon_b->fd_fl = -1;

	return 0;

err_rec5:
	close(fd_dev);

err_rec4:
	free(region);
	
err_rec3:
	close(daemon_b->fd_synth);
	daemon_b->fd_synth = -1;
	
err_rec2:
	close(daemon_b->fd_fl);
	daemon_b->fd_fl = -1;
	
err_rec1:
	return ret;
}
