#include <linux/dmaengine.h>
#include <linux/pagemap.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/memcontrol.h>
#include <linux/hrtimer.h>

#define MAX_QUANTUM 2 
struct list_dma_chan_prio list_chan_prio;
EXPORT_SYMBOL(list_chan_prio);

static struct priority_timer prio_timer;
typedef void (*prio_ptr_func)(struct list_dma_chan_prio * , unsigned short int);

/** 
 * priority_timer - struct for hrtimer
 * @list_chan: list priority channels
 * @timer: hrtimer
 * @tick: tick for change priority
 * @overruns_2: number overruns > 1
 * @quantum: time intervals of the priority
 * @prio_func_array: functions associated with the priority
*/ 
struct priority_timer
{
	struct list_dma_chan_prio *list_chan;
	struct hrtimer timer;
	unsigned short int tick;
	unsigned long overruns_2;
	ktime_t quantum[MAX_QUANTUM];
	prio_ptr_func prio_func_array[MAX_QUANTUM];
};

/* relase priority channels */
static void release_dma_chan_prio(void)
{
	uint i;
	for(i = 0; i < list_chan_prio.size ; ++i) 
	{	
		dma_release_channel(list_chan_prio.chan_prio[i]);
		list_chan_prio.chan_prio[i] = NULL;
	}
	list_chan_prio.size = 0;
}

/* init available priority channels */
static void init_dma_chan_prio(void)
{
	uint i;

	list_chan_prio.size = 0;
	for(i = 0; i < DMA_MAX_CHAN_PRIO ; ++i) 
	{
		list_chan_prio.chan_prio[i] = NULL;
		list_chan_prio.priority[i] = DMA_LOW_PRIO;
	}
}

/* make available priority channels */
static int complete_dma_chan_prio(void)
{
	uint i;
	dma_cap_mask_t mask;
	
	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	
	for(i = 0; i < DMA_MAX_CHAN_PRIO ; ++i) 
	{
	
		list_chan_prio.chan_prio[i] = dma_request_channel(mask, NULL , NULL);

		if(list_chan_prio.chan_prio[i])
		{
			++list_chan_prio.size;
		} 
		else
		{
			release_dma_chan_prio();
			printk(KERN_WARNING "ioat: WARNING no DMA chan alloc (priority chan policy)\n");
			return -1;
		}
	}	

	printk(KERN_DEBUG "ioat: DMA chan alloc (priority chan policy)\n");
	return 0;
}

/**
 * resume_chan_prio - resume the DMA channels
 * @list_prio: list priority channels
 * @priority: resume the channels that have the priority < @priority
 * Warning: priority 0 >  priority 1
*/ 
static void resume_chan_prio(struct list_dma_chan_prio *list_prio , unsigned short int priority)
{
	int i;
	for ( i = 0 ; i < list_prio->size ; ++i)
	{
		if(list_prio->priority[i] > priority)
		{
			ioat_resume_chan(list_prio->chan_prio[i]);
		}
		
	}
}

/**
 * suspend_chan_prio - suspend the DMA channels
 * @list_prio: list priority channels
 * @priority: suspend the channels that have the priority < @priority
 * Warning: priority 0 >  priority 1
*/ 
static void supend_chan_prio(struct list_dma_chan_prio *list_prio , unsigned short int priority)
{
	int i;
	for ( i = 0 ; i < list_prio->size  ; ++i)
	{
		if(list_prio->priority[i] > priority)
		{
			ioat_suspend_chan(list_prio->chan_prio[i]);
		}
	}
} 
 
/* callback hrtimer */
static enum hrtimer_restart prio_hrtimer_callback(struct hrtimer *timer)
{
 struct priority_timer *prio;
 u64 ret;
 
 prio = container_of(timer, struct priority_timer , timer);
 
 prio->tick = prio->tick % MAX_QUANTUM;
 prio->prio_func_array[prio->tick](prio->list_chan , DMA_HIGH_PRIO);
 
 ret = hrtimer_forward_now(timer, prio->quantum[prio->tick]);
 ++prio->tick;
 
 if(ret > 1)
	++prio->overruns_2;
 
 return HRTIMER_RESTART;
}       

/**
 * start_policy_prio2 - start the priority dynamic policy with two quantum.
 * @nanosecs_high: high priority quantum 
 * @nanosecs_low: low priority quantum 
 * return: the number of channels allocated or negative number for error
 */
int start_policy_prio2(long nanosecs_high , long nanosecs_low)
{
	int  i;
	unsigned short int priority;
	
	init_dma_chan_prio();
	
	if(complete_dma_chan_prio() < 0)
		goto no_prio_channel;
	
	priority = DMA_HIGH_PRIO;
	for(i = 0; i < list_chan_prio.size ; ++i) 
	{
		list_chan_prio.priority[i] = priority;
		++priority;
	}
	
	prio_timer.list_chan = &list_chan_prio;
	prio_timer.tick = (unsigned long long) MAX_QUANTUM;
	prio_timer.overruns_2 = 0;
	prio_timer.quantum[0] = ktime_set( 0 , nanosecs_high);
	prio_timer.quantum[1] = ktime_set( 0 , nanosecs_low);
	prio_timer.prio_func_array[0] = supend_chan_prio;
	prio_timer.prio_func_array[1] = resume_chan_prio;
	
	hrtimer_init(&prio_timer.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	prio_timer.timer.function = prio_hrtimer_callback;
	
	hrtimer_start(&prio_timer.timer, prio_timer.quantum[0] , HRTIMER_MODE_REL);
	
	return list_chan_prio.size;
	
no_prio_channel:
	return -1;
}
EXPORT_SYMBOL(start_policy_prio2);

/* stop priority dynamic policy */
void stop_policy_prio(void)
{
  hrtimer_cancel(&prio_timer.timer);
  resume_chan_prio(&list_chan_prio , DMA_HIGH_PRIO);
  prio_timer.list_chan = NULL;
  prio_timer.tick = 0;
  release_dma_chan_prio();
  printk(KERN_DEBUG "ioat: DMA chan remove (priority chan policy)\n");
	printk(KERN_DEBUG "ioat: overruns > 1: %lu \n" , prio_timer.overruns_2);
}
EXPORT_SYMBOL(stop_policy_prio);

int __dma_copy_from_user_prio(struct request_copy_prio *req_cpy_local)
{
	int byte_offset;
	int copy;
	int page_index;
	void __kernel  *tmp_addr_kernel;
	void __user *tmp_addr_user;
	size_t tmp_len;
	
	tmp_addr_kernel = req_cpy_local->kernel_base;
	tmp_addr_user = req_cpy_local->user_base;
	tmp_len = req_cpy_local->len;

	byte_offset = ((unsigned long)req_cpy_local->user_base & ~PAGE_MASK);
	page_index = 0 ;
	
	copy = min_t(int, PAGE_SIZE - byte_offset, tmp_len);

	/* break up copies to not cross page boundary */
	while(1)
	{		
		req_cpy_local->dma_cookie = dma_async_memcpy_pg_to_buf(req_cpy_local->chan, 
			req_cpy_local->pages[page_index], byte_offset ,tmp_addr_kernel, copy);
		
		/* poll for a descriptor slot */
		if (unlikely(req_cpy_local->dma_cookie < 0)) 
		{
			dma_async_issue_pending(req_cpy_local->chan);
			continue;	
		}
		
		tmp_len -= copy;
		if(!tmp_len)
			goto end_copy;
		
		tmp_addr_user += copy;
		tmp_addr_kernel += copy;
		byte_offset = 0;
		page_index++;
		copy = min_t(int, PAGE_SIZE, tmp_len);
	}

end_copy:
	req_cpy_local->status = dma_sync_wait(req_cpy_local->chan, req_cpy_local->dma_cookie);

	return tmp_len;
}
EXPORT_SYMBOL(__dma_copy_from_user_prio);

/** 
 * copy_from_user with DMA engine without pin/unpin (dynamic priority policy)
 * return: 0 for sucessfull, negative for error  
 * and positive for byte again to copy 
 * */ 
int _dma_copy_from_user_prio(struct request_copy_prio *req_cpy_local)
{
	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_prio(req_cpy_local->priority);	
	
		if (req_cpy_local->chan == NULL )
		{
			return -EBUSY;
		}
		return  __dma_copy_from_user_prio(req_cpy_local);
	}
	return 0;
}
EXPORT_SYMBOL(_dma_copy_from_user_prio);



int __dma_copy_to_user_prio(struct request_copy_prio *req_cpy_local)
{
	int byte_offset;
	int copy;
	int page_index;
	void __kernel  *tmp_addr_kernel;
	void __user *tmp_addr_user;
	size_t tmp_len;

	tmp_addr_kernel = req_cpy_local->kernel_base;
	tmp_addr_user = req_cpy_local->user_base;
	tmp_len = req_cpy_local->len;

	byte_offset = ((unsigned long)req_cpy_local->user_base & ~PAGE_MASK);
	page_index = 0 ;
	
	copy = min_t(int, PAGE_SIZE - byte_offset, tmp_len);

	/* break up copies to not cross page boundary */
	while(1)
	{				
		req_cpy_local->dma_cookie = dma_async_memcpy_buf_to_pg(req_cpy_local->chan, 
			req_cpy_local->pages[page_index], byte_offset ,tmp_addr_kernel, copy);

		/* poll for a descriptor slot */
		if (unlikely(req_cpy_local->dma_cookie < 0)) 
		{
			dma_async_issue_pending(req_cpy_local->chan);
			continue;	
		}
		
		tmp_len -= copy;
		if(!tmp_len)
			goto end_copy;
		
		tmp_addr_user += copy;
		tmp_addr_kernel += copy;
		byte_offset = 0;
		page_index++;
		copy = min_t(int, PAGE_SIZE, tmp_len);
	}

end_copy:
	req_cpy_local->status = dma_sync_wait(req_cpy_local->chan, req_cpy_local->dma_cookie);

	return tmp_len;
}
EXPORT_SYMBOL(__dma_copy_to_user_prio);

/** 
 * copy_to_user with DMA engine without pin/unpin (dynamic priority policy)
 * return: 0 for sucessfull, negative for error 
 * and positive for byte again to copy
 */ 
int _dma_copy_to_user_prio(struct request_copy_prio *req_cpy_local)
{
 	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_prio(req_cpy_local->priority);	
		
		if (req_cpy_local->chan == NULL )
			return -EBUSY;
			
		return __dma_copy_to_user_prio(req_cpy_local);
	}
	return 0;
}
EXPORT_SYMBOL(_dma_copy_to_user_prio);


/*
 * Pin down all the user pages needed for req_cp->len bytes.
*/
int dma_pin_user_pages_prio(struct request_copy_prio *req_cp , int write , int force)
{
	int ret;

	/* determine how many pages there are, up front */
	req_cp->nr_pages = ((PAGE_ALIGN((unsigned long)req_cp->user_base + req_cp->len) - 
		((unsigned long)req_cp->user_base  & PAGE_MASK)) >> PAGE_SHIFT);

	/* single kmalloc for pinned list, page_list[], and the page arrays */
	req_cp->pages = kmalloc((sizeof(struct page*) * req_cp->nr_pages), GFP_KERNEL);
	if (!req_cp->pages)
		goto out;

	/* pin pages down */
	down_read(&current->mm->mmap_sem);
	ret = get_user_pages( current, current->mm , (unsigned long) req_cp->user_base, req_cp->nr_pages,
			write ,	/* write */  force,	/* force */	req_cp->pages,	NULL);
	up_read(&current->mm->mmap_sem);

	if (ret != req_cp->nr_pages)
		goto unpin;

	return PIN_SUCCESSFUL;

unpin:
	dma_unpin_user_pages_prio(req_cp , SET_DIRTY_OFF);
out:
	return PIN_FAILED;
}
EXPORT_SYMBOL(dma_pin_user_pages_prio);

/*
 * Pin up all the user pages in the req_cp->pages.
*/
void dma_unpin_user_pages_prio(struct request_copy_prio *req_cp , int dirty)
{
	int i ;
	if( req_cp->pages != NULL )
	{
		for (i = 0; (i < req_cp->nr_pages) && req_cp->pages[i] != NULL ; i++) 
		{
			if(dirty)
			set_page_dirty_lock(req_cp->pages[i]);
			
			page_cache_release(req_cp->pages[i]);
		}
		kfree(req_cp->pages);
	}
}
EXPORT_SYMBOL(dma_unpin_user_pages_prio);
