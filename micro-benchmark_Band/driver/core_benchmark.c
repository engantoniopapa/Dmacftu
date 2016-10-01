#include "micro_benchmark.h"

extern short int type_measure;


/**
 * clean struct benchmark_output
 * @mb: struct benchmark_dev* mb
*/
void clean_mb_data(struct benchmark_dev* mb)
{	
	kfree(mb->data);
}

/**
 * init a struct benchmark_dev
 * @mb: struct device state
 * @name: name of the device
 * @k_addr: start address of kernel memory region
 * @k_size: size (in byte) of kernel memory region
 * @type_id: type id operation
 * @id_chan: id for DMA channel
 * Returns -ENOMEM for error. On success, this will be zero.

*/
int init_micro_bench(struct benchmark_dev* mb , char* name , char* k_addr , 
										 unsigned long k_size , short int type_id , short int id_chan)
{	
	mb->data = (struct benchmark_data*) kzalloc( sizeof(struct benchmark_data)  , GFP_KERNEL);
	if (!mb->data)
	{
		printk(KERN_WARNING "%s: Error kzalloc() for struct benchmark_data\n" , DRIVER_NAME);
		return -ENOMEM;
	}
	
	memset(mb->name , '\0' , MAX_CHAR_NAME);
	strncpy(mb->name , name , MAX_CHAR_NAME-1);
	mb->devt = 0;
	
	mb->data->type = type_id;
	mb->data->user_buf = NULL;
	mb->data->user_size = 0;
	mb->data->id_chan = id_chan;
	mb->data->kernel_buf = k_addr;
	mb->data->kernel_size = k_size;
	printk(KERN_INFO "%s: chan %d kernel space addr: %p , size: %lu \n" , 
				 DRIVER_NAME, id_chan , k_addr , k_size);
	return 0;
}


