#include <linux/bootmem.h>
#include <linux/module.h>

char *boot_mem_microBenchmark;
EXPORT_SYMBOL(boot_mem_microBenchmark);

unsigned long size_boot_mem_mb;
EXPORT_SYMBOL(size_boot_mem_mb);

/* init kernel memory region at boot system */
void __init boot_init_mem_mb(void)
{
#ifdef CONFIG_MEM_MICRO_BENCHMARK_32
	size_boot_mem_mb = 33554432;	
#endif

#ifdef CONFIG_MEM_MICRO_BENCHMARK_64
	size_boot_mem_mb = 67108864;
#endif	

#ifdef CONFIG_MEM_MICRO_BENCHMARK_128
	size_boot_mem_mb = 134217728;
#endif

#ifdef CONFIG_MEM_MICRO_BENCHMARK_256
	size_boot_mem_mb = 268435456;
#endif

#ifdef CONFIG_MEM_MICRO_BENCHMARK_512
	size_boot_mem_mb = 536870912;
#endif

#ifdef CONFIG_MEM_MICRO_BENCHMARK_1024
	size_boot_mem_mb = 1073741824;
#endif

	boot_mem_microBenchmark = alloc_bootmem(size_boot_mem_mb);
	
	if(!boot_mem_microBenchmark)
	{
		printk(KERN_WARNING "mem_mb: Error alloc_bootmem() of %lu byte !!!!\n" , size_boot_mem_mb);
	}
	else
	{
		printk(KERN_DEBUG "mem_mb: alloc_bootmem() of %lu byte \n" , size_boot_mem_mb);
	}
}


