#include <linux/dmaengine.h>
#include <linux/module.h>

#ifdef CONFIG_EXCL_COPY_TO_FROM_IOATDMA
#include <linux/percpu.h>

DEFINE_PER_CPU(struct dma_chan_copy , percpu_chan_copy);
EXPORT_PER_CPU_SYMBOL(percpu_chan_copy);

/* relase exclusive channels */
static void boot_release_dma_chan(void)
{ 
  uint i;
	struct dma_chan_copy *cpu_chan;

	for_each_present_cpu(i) 
	{
		cpu_chan = &per_cpu(percpu_chan_copy , i);
		
		if(cpu_chan->use == 1)
		{
			dma_release_channel(cpu_chan->chan);
			cpu_chan->chan = NULL;
			cpu_chan->use = 0;
		} 
	}
}

/* init available exclusive channels */
void __init boot_init_dma_chan(void)
{ 
	uint i;
	struct dma_chan_copy *cpu_chan;

	for_each_present_cpu(i) 
	{
		cpu_chan = &per_cpu(percpu_chan_copy , i);
		cpu_chan->use = 0;
		cpu_chan->chan = NULL;
	}
}

/* make available exclusive channels */
void __init boot_complete_dma_chan(void)
{
	uint i;
	dma_cap_mask_t mask;
	struct dma_chan *chan;
	struct dma_chan_copy *cpu_chan;
	
	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	
	for_each_present_cpu(i) 
	{
		cpu_chan = &per_cpu(percpu_chan_copy , i);
		chan = dma_request_channel(mask, NULL , NULL);

		if(chan)
		{
			cpu_chan->use = 1;
			cpu_chan->chan = chan;
		} 
		else
		{
			goto error_boot;
		}
	}
	printk(KERN_DEBUG "ioat: DMA chan alloc (exclusive chan policy)\n");
	
	return;

error_boot:
	boot_release_dma_chan();
	printk(KERN_WARNING "ioat: WARNING no DMA chan alloc (exclusive chan policy)\n");
  return;
}
#endif

#ifdef CONFIG_SHARED_COPY_TO_FROM_IOATDMA

void __init boot_init_dma_chan(void)
{
	
}
/* make available shared channels */
void __init boot_complete_dma_chan(void)
{
	copy_dmaengine_get();
	printk(KERN_DEBUG "ioat: DMA chan alloc (shared chan policy)\n");
}

#endif

#ifdef CONFIG_DYNAMIC_CHAN_POLICY_IOATDMA

void __init boot_init_dma_chan(void)
{

}

void __init boot_complete_dma_chan(void)
{
}
#endif


