#ifndef _CORE_BENCHMARK_H_
#define _CORE_BENCHMARK_H_

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif
void clean_mb_results(struct benchmark_dev* mb);

int init_micro_bench(struct benchmark_dev* mb , char* name , char* k_addr 
		, unsigned long k_size , short int type_id);

int start_measure(struct benchmark_data *b_data ,
	 		int (*ptr_copy)(struct unit_buff_user* , struct request_copy*, short int ),
			short int mb_cache);

int init_measure(struct benchmark_data *b_data);

#endif
