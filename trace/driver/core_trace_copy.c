#include "trace_copy.h"

extern int count_region;

/**
 * clean struct region_mem
 * @t_dev: struct trace_dev
*/
void clean_trace_region_mem(struct trace_dev *t_dev)
{	
	kfree(t_dev->data.ptr_region);
}

/**
 * init a struct trace_dev
 * @trace_dev: the struct trace_dev to initialized
 * @cb: struct CircularBuffer for struct trace_data
 * @name: name of the device
 * Returns the 0 or -ENOMEM for error 
*/
int init_trace_dev(struct trace_dev *t_dev ,struct CircularBuffer *cb , char *name)
{	
	
	t_dev->data.ptr_region = (struct region_mem *) kzalloc (	(sizeof(struct region_mem) * count_region) , GFP_KERNEL);
	if (!t_dev->data.ptr_region)
		goto err_nomem_t_dev1;
	
	t_dev->data.ptr_cb = cb;
	
	memset(t_dev->name , '\0' , MAX_CHAR_NAME);
	strncpy(t_dev->name , name , MAX_CHAR_NAME-1);
	t_dev->devt = 0;
	return 0;
	
err_nomem_t_dev1:
	return -ENOMEM;
}

/**
 * region_read_cpu - read max @len measures from @ptr_cb 
 * and copy them on @reg_mem. 
 * @len: max measures to read and copy
 * @ptr_cb: address to read the measures
 * @reg_m: address to copy the measures
 */
void region_read_cpu(int len , struct CircularBuffer *ptr_cb , struct region_mem *reg_m)
{
	reg_m->count = 0;

	/* read from kernel buffer max len element */
	len = cbRead_list(ptr_cb, (struct unit_buff *) &(reg_m->buffer[reg_m->count]) , len);
	reg_m->count = reg_m->count + len ;
}


