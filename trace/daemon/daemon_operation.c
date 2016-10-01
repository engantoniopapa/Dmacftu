#include "daemon.h"

/**
 * create sparse file -
 * @daemon_ptr: struct daemon_trace
 * @flags: flags for open()
 * @mode: mode for open()
 * return: zero on success or negative value for error
 */
int daemon_create_sparse_file(struct daemon_trace *daemon_ptr , 
		 int flags , mode_t mode)
{
	int ret;
	
	daemon_ptr->fd_sp = open(daemon_ptr->path_file_sp ,flags, mode);
	if(daemon_ptr->fd_sp < 0)
	{
		perror("open sparse file error");
		return daemon_ptr->fd_sp;
	}
	
	ret = lseek(daemon_ptr->fd_sp , daemon_ptr->size_max , SEEK_END);
	if( ret < 0)
	{
		perror("lseek sparse file error");
		goto err_file;
	}
	
	ret = write(daemon_ptr->fd_sp , "\0", 1);
	if(ret < 0)
	{
		perror("write sparse file error");
		goto err_file;
	}
	
	return 0;
	
err_file:
	close(daemon_ptr->fd_sp);
	return ret;
}

/** 
 * write first row in  sparse file, the # is used for gnuplot 
 * @daemon_ptr: struct daemon_trace
 * return: zero on success or negative value for error
 */	
int sparse_first_row(struct daemon_trace *daemon_ptr)
{
	int fd;
	char *map;

	fd = daemon_ptr->fd_sp;
	map = (char*) mmap(NULL , N_CHAR_LINE , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		
	if (map == MAP_FAILED) 
	{
		perror("Error mmap");
		return -EINVAL;
	}
	
	strncpy(map , "#[cpu]CounterEvent     Byte Copiati  end[cpu]     StartTime___(ns)       EndTime___(ns)   DiffTime(ns)             ReturnPoint      Type_op\n", N_CHAR_LINE );
	daemon_ptr->offset_map = strlen(map);	
	
	if (munmap(map, N_CHAR_LINE) == -1) 
	{
		perror("Error munmap");
		return -EINVAL;
	}
	return 0;
}

/** 
 * copy sparse file in the regular file
 * @daemon_ptr: struct daemon_trace
 * return: zero on success or negative value for error
 */	
int copy_normal_file(struct daemon_trace *ptr_daemon)
{
	int ret;
	char *buffer_file;
	long page_size = sysconf(_SC_PAGESIZE); //la size di una page nel mio sistema

	buffer_file = (char *) malloc ( page_size );
	if(!buffer_file)
		goto err_fill1;


	while(ptr_daemon->offset_map > 0 )
	{
		memset(buffer_file ,'\0', page_size);
		
		if(ptr_daemon->offset_map >= page_size)
		{
			ret = read(ptr_daemon->fd_sp , buffer_file , page_size);
			if(ret < 0)
				goto err_fill2;
				
			ret = write(ptr_daemon->fd_fl , buffer_file, ret);
			if(ret < 0)
				goto err_fill2;
				
			ptr_daemon->offset_map -= ret ;			
		}
		else
		{
			ret = read(ptr_daemon->fd_sp , buffer_file , ptr_daemon->offset_map);
			if(ret < 0)
				goto err_fill2;
			
			ret = write( ptr_daemon->fd_fl, buffer_file, ret);
			if(ret < 0)
				goto err_fill2;
				
			ptr_daemon->offset_map -= ret ;
		}
	}

free(buffer_file);
return 0;

err_fill2:
	free(buffer_file);
	
err_fill1:
	return ret;
}

