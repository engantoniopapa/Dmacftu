#include <linux/circular_buffer.h>
#include <linux/percpu.h>
#include <linux/module.h>

DEFINE_PER_CPU(struct CircularBuffer, mem_buff_to);
EXPORT_PER_CPU_SYMBOL(mem_buff_to);

DEFINE_PER_CPU(struct CircularBuffer, mem_buff_from);
EXPORT_PER_CPU_SYMBOL(mem_buff_from);

DEFINE_PER_CPU(struct CircularBuffer, __mem_buff_to);
EXPORT_PER_CPU_SYMBOL(__mem_buff_to);

DEFINE_PER_CPU(struct CircularBuffer, __mem_buff_from);
EXPORT_PER_CPU_SYMBOL(__mem_buff_from);

/* init trace for copy_to/from_user */
void __init boot_init_trace_copy(void)
{ 
  uint i;
  for_each_present_cpu(i) 
  {
		struct CircularBuffer *ptr = &per_cpu(mem_buff_to , i);
		cbInit(ptr);
		
		ptr = &per_cpu(mem_buff_from , i);
		cbInit(ptr);
		
		ptr = &per_cpu(__mem_buff_to , i);
		cbInit(ptr);
		
		ptr = &per_cpu(__mem_buff_from , i);
		cbInit(ptr);
  }
}
