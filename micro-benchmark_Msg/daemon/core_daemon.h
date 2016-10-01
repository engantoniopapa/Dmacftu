#ifndef _CORE_DAEMON_H_
#define _CORE_DAEMON_H_

struct region_mem * init_region(void);

struct daemon_benchmark *init_daemon_benchmark(void);

int	record_measures(struct daemon_benchmark *daeom_p , 
unsigned long max_size , char *path_dev);

#endif
