#include "trace_copy.h"

/*struct kernel for trace */
extern struct CircularBuffer __mem_buff_from ; 
extern struct CircularBuffer mem_buff_from ;
extern struct CircularBuffer __mem_buff_to ; 
extern struct CircularBuffer mem_buff_to ; 

/* struct state for device driver */
struct trace_dev vector_trace[TRACE_MINOR];

static dev_t base_dev; /*  first device number */
static struct class *trace_class; /* for device class */

/* number of regions, one for cpu */
int count_region;

/** 
 * init all struct trace_dev for driver - 
 * returns zero on success, or negative value for error.
 */
int init_all_trace(void)
{
	int ret;
	
	ret = init_trace_dev(&(vector_trace[0]) , &__mem_buff_from , "__mem_trace_from" ) ; 
	if(ret < 0)
		return -ENOMEM;
	
	ret = init_trace_dev( &(vector_trace[1]) , &__mem_buff_to , "__mem_trace_to"); 
	if(ret < 0)
		goto err_trace1;
		
	ret = init_trace_dev( &(vector_trace[2]) , &mem_buff_from , "mem_trace_from"); 
	if(ret < 0)
		goto err_trace2;
	
	ret = init_trace_dev( &(vector_trace[3]) , &mem_buff_to , "mem_trace_to"); 
	if(ret < 0)
		goto err_trace3;
	
	return 0;
	
err_trace3:
	clean_trace_region_mem( &(vector_trace[2]) );
		
err_trace2:
	clean_trace_region_mem( &(vector_trace[1]) );		
		
err_trace1:
	clean_trace_region_mem( &(vector_trace[0]) );
	return -ENOMEM;
}

/**
 * registers drivers and devices for module 
 * returns negative value for error, zero if the driver is register.
*/
int trace_register_driver(void) 
{
	int ret = 0;
	
	/* registers a range of char device numbers */
	ret = alloc_chrdev_region(&base_dev, 0, TRACE_MINOR , DRIVER_NAME);
	if(ret < 0) 
	{
		printk(KERN_WARNING "%s: Failed to allocate char device region\n" , DRIVER_NAME);   
		goto err_dev;
	}
	
	/* creates a struct class structure */
  trace_class = class_create(THIS_MODULE, DRIVER_NAME);
  
  if (IS_ERR(trace_class))
  {
		ret = PTR_ERR(trace_class);
    printk(KERN_WARNING "%s: Failed to create trace mem class\n" , DRIVER_NAME);
		goto err_class;
  }
  
  ret = trace_driver_add_chardev(vector_trace, TRACE_MINOR,	MAJOR(base_dev));
  if(ret < 0)
		goto err_cdev;
		
   /* creates the devices and registers them with sysfs */
  ret = trace_device_create(vector_trace, TRACE_MINOR , trace_class);
  if(ret < 0)
		goto err_dev_create;
   
	return 0;

err_dev_create:
	all_trace_driver_del_chardev(vector_trace, TRACE_MINOR );
	
err_cdev:
	class_destroy(trace_class); 
	
err_class:
	unregister_chrdev_region(base_dev, TRACE_MINOR);
	base_dev = 0;

err_dev:
	return ret;
}

/* Module Init */
static int __init mem_trace_init(void)
{  
	int i;
	int ret;
	printk("%s: Start Module \n", DRIVER_NAME);	
	
	trace_class = NULL;
	base_dev = 0 ;
	
	count_region = 0;
	for_each_present_cpu(i) 
	{
		++count_region;	
	}
	printk_once(KERN_INFO "%s: run on %u CPU\n" , DRIVER_NAME , count_region );

	/* alloc and init structure */
	ret = init_all_trace();
	if(ret < 0)
	{
		printk(KERN_WARNING "%s: Out of memory\n" , DRIVER_NAME);
		return ret;
	}
	
	/*register driver and create device */
	ret = trace_register_driver();
	if(ret == 0)
	{
		printk(KERN_INFO "%s: register  \n" , DRIVER_NAME);
		printk(KERN_INFO "%s: major number %d \n" , DRIVER_NAME , MAJOR(base_dev));
	}
	else
	{
		for( i = 0; i < TRACE_MINOR ; ++i)
			clean_trace_region_mem(&(vector_trace[i]) );		
	
		printk(KERN_INFO "%s: unregister \n" , DRIVER_NAME);
		return ret;
	}
	
	return 0;
}

/* Module Exit */
static void __exit mem_trace_exit(void)
{	
	int i ;		
	printk("%s: End Module \n" , DRIVER_NAME);

	/* start clean all */	
	all_trace_device_destroy(vector_trace, TRACE_MINOR , trace_class);
	all_trace_driver_del_chardev(vector_trace, TRACE_MINOR );
	
	class_destroy(trace_class); 
	unregister_chrdev_region(base_dev, TRACE_MINOR);
	
	for( i = 0; i < TRACE_MINOR ; ++i)
		clean_trace_region_mem(&(vector_trace[i]) );
	/* end clean all */
	
	return;
}

module_init(mem_trace_init);
module_exit(mem_trace_exit);

MODULE_AUTHOR("Antonio Papa");
MODULE_DESCRIPTION("trace for copy_to/from_user");
MODULE_LICENSE("GPL and additional rights");
MODULE_VERSION(TRACE_COPY_VERSION);
MODULE_ALIAS(DRIVER_NAME);
