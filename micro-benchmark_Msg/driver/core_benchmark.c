#include "micro_benchmark.h"

extern short int type_measure;


/**
 * clean struct benchmark_output
 * @mb: struct benchmark_dev* mb
*/
void clean_mb_results(struct benchmark_dev* mb)
{	
	kfree(mb->data.results);
}

/**
 * init a struct benchmark_dev
 * @mb: struct device state
 * @name: name of the device
 * @k_addr: start address of kernel memory region
 * @k_size: size (in byte) of kernel memory region
 * @type_id: type id operation
 * Returns 0 on success or -ENOMEM for error 
*/
int init_micro_bench(struct benchmark_dev* mb , char* name , char* k_addr , 
										 unsigned long k_size , short int type_id)
{	
	
	mb->data.results = (struct benchmark_output *) kzalloc( sizeof(struct benchmark_output)  , GFP_KERNEL);
	if (!mb->data.results)
	{
		printk(KERN_WARNING "%s: Error kzalloc() for struct benchmark_output\n" , DRIVER_NAME);
		return -ENOMEM;
	}
	
	memset(mb->name , '\0' , MAX_CHAR_NAME);
	strncpy(mb->name , name , MAX_CHAR_NAME-1);
	mb->devt = 0;
	
	mb->data.type = type_id;
	mb->data.user_buf = NULL;
	mb->data.user_size = 0;
	mb->data.results->count = 0;
	mb->data.kernel_buf = k_addr;
	mb->data.kernel_size = k_size;
	printk_once(KERN_INFO "%s: kernel space addr: %p , size: %lu \n" , DRIVER_NAME , k_addr , k_size);
	return 0;
}

/** 
 * operation of measure that does copy_from_user with CPU
 * @unit: struct to save the result
 * @req: struct for request copy
 * @mb_cache: 1 -> use cache , 0 -> not use cache; 
*/
static int cpu_copy_from_measure(struct unit_buff_user *unit ,struct request_copy *req , short int mb_cache)
{
	int ret;
		
	if(!mb_cache)
	{
		clflush_cache_range(req->kernel_base , req->len);
		clflush_cache_range(req->user_base , req->len);
	}
	
	mb() ;	
		unit->clock_cpu_start = cpu_clock(unit->index_cpu);
	mb() ;
	
	ret = copy_from_user_pure(req->kernel_base, req->user_base, req->len);
	
	mb() ;
		unit->clock_cpu_end = cpu_clock(unit->index_cpu);
	mb() ;	
	
	return ret;
}

/** 
 * operation of measure that does copy_to_user with CPU
 * @unit: struct to save the result
 * @req: struct for request copy
 * @mb_cache: 1 -> use cache , 0 -> not use cache; 
*/
static int cpu_copy_to_measure(struct unit_buff_user *unit ,struct request_copy *req, short int mb_cache)
{
	int ret;
		
	if(!mb_cache)
	{
		clflush_cache_range(req->kernel_base , req->len);
		clflush_cache_range(req->user_base , req->len);
	}
	
	mb() ;	
		unit->clock_cpu_start = cpu_clock(unit->index_cpu);
	mb() ;
	
	ret = copy_to_user_pure(req->user_base, req->kernel_base,  req->len);
	
	mb() ;
		unit->clock_cpu_end = cpu_clock(unit->index_cpu);
	mb() ;	
	
	return ret;
}

/** 
 * operation of measure that does copy_from_user with DMA (exclusive policy)
 * @unit: struct to save the result
 * @req: struct for request copy
 * @mb_cache: 1 -> use cache , 0 -> not use cache; 
*/
static int dma_copy_from_measure_ex(struct unit_buff_user *unit, struct request_copy *req , short int mb_cache)
{
	int ret;
	
	if(!mb_cache)
	{
		clflush_cache_range(req->kernel_base , req->len);
		clflush_cache_range(req->user_base , req->len);
	}
	
	mb() ;	
		unit->clock_cpu_start = cpu_clock(unit->index_cpu);
	mb() ;
	
	ret = _dma_copy_from_user_excl(req);

	mb() ;
		unit->clock_cpu_end = cpu_clock(unit->index_cpu);
	mb() ;	

	return ret;
}

/** 
 * operation of measure that does copy_to_user with DMA (exclusive policy)
 * @unit: struct to save the result
 * @req: struct for request copy
 * @mb_cache: 1 -> use cache , 0 -> not use cache; 
*/
static int dma_copy_to_measure_ex(struct unit_buff_user *unit ,struct request_copy *req , short int mb_cache)
{
	int ret;
	
	if(!mb_cache)
	{
		clflush_cache_range(req->kernel_base , req->len);
		clflush_cache_range(req->user_base , req->len);
	}
	
	mb() ;	
		unit->clock_cpu_start = cpu_clock(unit->index_cpu);
	mb() ;
	
	ret = _dma_copy_to_user_excl(req);
	
	mb() ;
		unit->clock_cpu_end = cpu_clock(unit->index_cpu);
	mb() ;	
	
	return ret;
}

/** 
 * operation of measure that does copy_from_user with DMA (shared policy)
 * @unit: struct to save the result
 * @req: struct for request copy
 * @mb_cache: 1 -> use cache , 0 -> not use cache; 
*/
static int dma_copy_from_measure_sh(struct unit_buff_user *unit ,struct request_copy *req , short int mb_cache)
{
	int ret;
	
	if(!mb_cache)
	{
		clflush_cache_range(req->kernel_base , req->len);
		clflush_cache_range(req->user_base , req->len);
	}
	
	mb() ;	
		unit->clock_cpu_start = cpu_clock(unit->index_cpu);
	mb() ;
	
	ret = _dma_copy_from_user_sh(req);

	mb() ;
		unit->clock_cpu_end = cpu_clock(unit->index_cpu);
	mb() ;	

	return ret;
}

/** 
 * operation of measure that does copy_to_user with DMA (shared policy)
 * @unit: struct to save the result
 * @req: struct for request copy
 * @mb_cache: 1 -> use cache , 0 -> not use cache; 
*/
static int dma_copy_to_measure_sh(struct unit_buff_user *unit ,struct request_copy *req , short int mb_cache)
{
	int ret;
	
	if(!mb_cache)
	{
		clflush_cache_range(req->kernel_base , req->len);
		clflush_cache_range(req->user_base , req->len);
	}
	
	mb() ;	
		unit->clock_cpu_start = cpu_clock(unit->index_cpu);
	mb() ;
	
	ret = _dma_copy_to_user_sh(req);
	
	mb() ;
		unit->clock_cpu_end = cpu_clock(unit->index_cpu);
	mb() ;	
	
	return ret;
}

/** 
 * start the process of measure for a size 
 * @b_data: the details of the measures
 * @ptr_copy: type operation to test
 * @mb_cache: 1 -> use cache , 0 -> not use cache; 
 * return: zero on success or -EACCES for error
*/
int start_measure(struct benchmark_data *b_data ,
	 		int (*ptr_copy)(struct unit_buff_user* , struct request_copy*, short int ),
			short int mb_cache)
{
	int i;
	struct unit_buff_user *unit;
	struct request_copy req_cpy_local;
	int ret;
		
	init_req_copy(&req_cpy_local , (void *) b_data->user_buf, (void *) b_data->kernel_buf, b_data->user_size);
	b_data->results->count = 0;
	
	/* pin user pages for DMA */
	if(type_measure == _EXCLUSIVE_MEASURE || type_measure == _SHARED_MEASURE)
	{
		if(b_data->type == ID_COPY_FROM || b_data->type == ID_COPY_FROM_NC)
			ret = dma_pin_user_pages(&req_cpy_local , PIN_READ , 0);
		else
			ret = dma_pin_user_pages(&req_cpy_local , PIN_WRITE , 0) ;

		if(ret != PIN_SUCCESSFUL)
		{
			printk(KERN_WARNING "%s: Error Pin user pages\n" , DRIVER_NAME);
			return -EACCES;
		}
	}
	
	/* to cache */
	unit = &(b_data->results->buffer[0]);
	unit->index_cpu = get_cpu();	
	ret = (*ptr_copy)(unit , &req_cpy_local,  mb_cache);
	put_cpu();
	schedule();	
		
	for( i = 0 ; i < BUFF_MEASURE; ++i)
	{
		unit = &(b_data->results->buffer[i]);
		unit->index_cpu = get_cpu();	
		ret = (*ptr_copy)(unit , &req_cpy_local,  mb_cache);
		put_cpu();
		unit->size = req_cpy_local.len - ret;
		b_data->results->count++;
		schedule();	
	}
	
	/* unpin user pages for DMA*/
	if(type_measure == _EXCLUSIVE_MEASURE || type_measure == _SHARED_MEASURE)
	{
		if(b_data->type == ID_COPY_FROM || b_data->type == ID_COPY_FROM_NC)
			dma_unpin_user_pages(&req_cpy_local , SET_DIRTY_OFF);
		else
			dma_unpin_user_pages(&req_cpy_local , SET_DIRTY_ON);
	}
	return 0;
}

/** 
 * init the process of measure for a size
 * @b_data: the details of the measures
 * return: zero on success or negative number for error
*/
int init_measure(struct benchmark_data *b_data)
{
	short int mb_cache = 1;
	int ret;
	int (*ptr_copy)(struct unit_buff_user* , struct request_copy*, short int );

	if(!b_data->kernel_buf)
	 return -ENOMEM;
	 
	switch (b_data->type) 
	{
		case ID_COPY_FROM:
		{
			if(type_measure == _CPU_MEASURE)
				ptr_copy = cpu_copy_from_measure;
			
			if(type_measure == _EXCLUSIVE_MEASURE)
				ptr_copy = dma_copy_from_measure_ex;
			
			if(type_measure == _SHARED_MEASURE)
				ptr_copy = dma_copy_from_measure_sh;
		}
		break;
		
		case ID_COPY_TO:
		{
			if(type_measure == _CPU_MEASURE)
			ptr_copy = cpu_copy_to_measure;
			
			if(type_measure == _EXCLUSIVE_MEASURE)
				ptr_copy = dma_copy_to_measure_ex;
			
			if(type_measure == _SHARED_MEASURE)
				ptr_copy = dma_copy_to_measure_sh;
		}
		break;
		
		case ID_COPY_FROM_NC:
		{
			mb_cache = 0;

			if(type_measure == _CPU_MEASURE)
				ptr_copy = cpu_copy_from_measure;
					
			if(type_measure == _EXCLUSIVE_MEASURE)
				ptr_copy = dma_copy_from_measure_ex;
			
			if(type_measure == _SHARED_MEASURE)
				ptr_copy = dma_copy_from_measure_sh;

		}
		break;
		
		case ID_COPY_TO_NC:
		{
			mb_cache = 0;

			if(type_measure == _CPU_MEASURE)
				ptr_copy = cpu_copy_to_measure;
						
			if(type_measure == _EXCLUSIVE_MEASURE)
				ptr_copy = dma_copy_to_measure_ex;
			
			if(type_measure == _SHARED_MEASURE)
				ptr_copy = dma_copy_to_measure_sh;
		}
		break;
		
		default:
		{
			printk(KERN_WARNING "%s: Error type operation\n" , DRIVER_NAME);
			goto error_op;
		}
		break;
	} 

	ret = start_measure(b_data , ptr_copy , mb_cache);
	return ret;
	
error_op:
	return -EINVAL;
}

