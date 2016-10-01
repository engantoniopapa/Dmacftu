#include "micro_benchmark.h"

/* struct state for device driver */
struct benchmark_dev vector_mb[MB_MINOR];
static dev_t base_dev; /*  first device number */
static struct class *micro_bench_class; /* for device class */

/* for test in kernel space */
extern char *boot_mem_microBenchmark;
extern unsigned long size_boot_mem_mb;  

/* type measure:
 * 0 - cpu meausre
 * 1 - dma measure (exclusive policy)
 * 2 - dma measure (shared policy)
*/
short int type_measure = -1;
module_param(type_measure, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

/** 
 * init all struct benchmark_dev for driver - 
 * returns zero on success, or negative value for error.
 */
int init_all_micro_bench(void)
{
	int ret;
	
	ret = init_micro_bench(&(vector_mb[0]), "micro-benchmark_from" ,
				boot_mem_microBenchmark , size_boot_mem_mb , ID_COPY_FROM);
	if(ret < 0)
		return -ENOMEM;
		
	ret = init_micro_bench(&(vector_mb[1]), "micro-benchmark_to" ,
				boot_mem_microBenchmark , size_boot_mem_mb , ID_COPY_TO);
	if(ret < 0)
		goto err_mb1;
	
	ret = init_micro_bench(&(vector_mb[2]), "micro-benchmark_from_nc" ,
				boot_mem_microBenchmark , size_boot_mem_mb , ID_COPY_FROM_NC);
	if(ret < 0)
		goto err_mb2;
		
	ret = init_micro_bench(&(vector_mb[3]), "micro-benchmark_to_nc" ,
				boot_mem_microBenchmark , size_boot_mem_mb , ID_COPY_TO_NC);
	if(ret < 0)
		goto err_mb3;
		
	return 0;
	
err_mb3:
	clean_mb_results(&(vector_mb[2]) );
			
err_mb2:
	clean_mb_results(&(vector_mb[1]) );
	
err_mb1:
	clean_mb_results(&(vector_mb[0]) );
	return -ENOMEM;
}

/**
 * registers drivers and devices for module 
 * @mb: struct device state
 * Returns negative value for error, zero if the driver is register.
*/
int micro_bench_register_driver(void) 
{
	int ret;
	
	/* registers a range of char device numbers */
	ret = alloc_chrdev_region(&base_dev, 0, MB_MINOR , DRIVER_NAME);
	if(ret < 0) 
	{
		printk(KERN_WARNING "%s: Failed to allocate char device region\n" , DRIVER_NAME);   
		base_dev = 0;
		goto err_dev;
	}
	
	/* creates a struct class structure */
  micro_bench_class = class_create(THIS_MODULE, DRIVER_NAME);
  
  if (IS_ERR(micro_bench_class))
  {
		ret = PTR_ERR(micro_bench_class);
    printk(KERN_WARNING "%s: Failed to create benchmark class\n" , DRIVER_NAME);
		goto err_class;
  }
  
  ret = mb_driver_add_chardev(vector_mb , MB_MINOR,	MAJOR(base_dev));
  if(ret < 0)
		goto err_cdev;
		
  /* creates the devices and registers them with sysfs */
  ret = mb_device_create(vector_mb, MB_MINOR , micro_bench_class);
  if(ret < 0)
		goto err_dev_create;
  
	return 0;

err_dev_create:
	all_mb_driver_del_chardev(vector_mb, MB_MINOR);
	
err_cdev:
	class_destroy(micro_bench_class); 
	
err_class:
	unregister_chrdev_region(base_dev, MB_MINOR);
	base_dev = 0;

err_dev:
	return ret;
}

/* Module Init */
static int __init micro_benchmark_init(void)
{ 
	int ret; 
	int i;
	
	printk(KERN_INFO "%s: Start Module \n", DRIVER_NAME);	
	
	if(!boot_mem_microBenchmark || size_boot_mem_mb <= 0 )
	{
		printk(KERN_WARNING "%s: Error Kernel Boot Memory\n", DRIVER_NAME);
		return -ENOMEM;
	}
	
	printk(KERN_INFO "%s: Max size (in byte) for test %lu \n", DRIVER_NAME , size_boot_mem_mb);	

	switch (type_measure) 
	{
		case _CPU_MEASURE:
		{
			printk(KERN_INFO "%s: Micro-Benchmark on CPU \n", DRIVER_NAME);	
		}
		break;
		
		case _EXCLUSIVE_MEASURE:
		{
			if( start_policy_excl() < 0)
			{
				printk(KERN_WARNING "%s: Error start exclusive policy\n", DRIVER_NAME);
				return -EPERM;
			}
			printk(KERN_INFO "%s: Micro-Benchmark on DMA (exclusive policy)  \n", DRIVER_NAME);	
		}
		break;
		
		case _SHARED_MEASURE:
		{
			start_policy_sh();
			printk(KERN_INFO "%s: Micro-Benchmark on CPU (shared policy) \n", DRIVER_NAME);	
		}
		break;
		
		default:
		{
			printk(KERN_WARNING "%s: Error module input\n", DRIVER_NAME);
			return -EPERM;
		}
		break;
	}
	
	base_dev = 0 ;
	
	if(init_all_micro_bench())
		return -ENOMEM;
	
	/*register driver and create device */
	ret = micro_bench_register_driver();
	if(ret == 0)
	{
		printk(KERN_INFO "%s: register  \n" , DRIVER_NAME);
		printk(KERN_INFO "%s: major number %d \n" , DRIVER_NAME , MAJOR(base_dev));
	}
	else
	{	
		for( i = 0; i < MB_MINOR; ++i)
			clean_mb_results(&(vector_mb[i]) );
		
		printk(KERN_INFO "%s: unregister \n" , DRIVER_NAME);
		return ret;
	}
	
	return 0;
}

/* Module Exit */
static void __exit micro_benchmark_exit(void)
{
	int i ;
				
	printk("%s: End Module \n" , DRIVER_NAME);
	
	if(type_measure == _EXCLUSIVE_MEASURE)
		stop_policy_excl();
	
	if(type_measure == _SHARED_MEASURE)
		stop_policy_sh();
		
	/* start clean all */	
	all_mb_device_destroy(vector_mb, MB_MINOR , micro_bench_class);
	all_mb_driver_del_chardev(vector_mb, MB_MINOR);

	class_destroy(micro_bench_class); 
	unregister_chrdev_region(base_dev, MB_MINOR);
	
	for( i = 0; i < MB_MINOR; ++i)
		clean_mb_results(&(vector_mb[i]) );
	/* end clean all */

	return;
}

module_init(micro_benchmark_init);
module_exit(micro_benchmark_exit);

MODULE_AUTHOR("Antonio Papa");
MODULE_DESCRIPTION("micro-benchmark copy_to/from_user");
MODULE_LICENSE("GPL and additional rights");
MODULE_VERSION(BENCHMARK_VERSION);
MODULE_ALIAS(DRIVER_NAME);
