#ifndef _CORE_BENCHMARK_H_
#define _CORE_BENCHMARK_H_

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif
void clean_mb_data(struct benchmark_dev* mb);

int init_micro_bench(struct benchmark_dev* mb , char* name , char* k_addr 
		, unsigned long k_size , short int type_id , short int id_chan);
#endif
