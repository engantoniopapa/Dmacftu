#ifndef _CORE_DAEMON_H_
#define _CORE_DAEMON_H_

void del_region(struct region_mem *region);

struct region_mem * init_region(int n_cpu);

void init_daemon(struct daemon_trace *daemon_ptr ,int n_cpu , int n_read , char *dev_name);

int clean_kernel_buffer(int fd_dev , struct region_mem *region , size_t size_r);

unsigned long long diff_time(unsigned long long start , unsigned long long end);

int save_measure(struct daemon_trace *daemon_ptr , int n_cpu , 
		struct region_mem *region, int fd_dev);

int	record_measures(struct daemon_trace *daemon_ptr , int n_cpu , 
		unsigned long n_read , unsigned long delay , char *path_dev);

#endif
