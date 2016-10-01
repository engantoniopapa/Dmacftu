#include <linux/dmaengine.h>
#include <linux/pagemap.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/memcontrol.h>
#include <linux/percpu.h>

#ifdef CONFIG_POLICY_IOATDMA

	#ifdef CONFIG_EXCL_COPY_TO_FROM_IOATDMA
		extern struct dma_chan_copy percpu_chan_copy;
	#else /* CONFIG_EXCL_COPY_TO_FROM_IOATDMA */
		DEFINE_PER_CPU(struct dma_chan_copy , percpu_chan_copy);
		EXPORT_PER_CPU_SYMBOL(percpu_chan_copy);
	#endif /* CONFIG_EXCL_COPY_TO_FROM_IOATDMA */
	
#endif /* CONFIG_POLICY_IOATDMA */

/* relase exclusive channels */
static void release_dma_chan_excl(void)
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
static void init_dma_chan_excl(void)
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
static int complete_dma_chan_excl(void)
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
			release_dma_chan_excl();
			printk(KERN_WARNING "ioat: WARNING no DMA chan alloc (exclusive chan policy)\n");
			return -1;
		}
	}
	printk(KERN_DEBUG "ioat: DMA chan alloc (exclusive chan policy)\n");
	return 0;
}

/* start exclusive dynamic policy */
int start_policy_excl(void)
{
	init_dma_chan_excl();
	return complete_dma_chan_excl();
}
EXPORT_SYMBOL(start_policy_excl);

/* stop exclusive dynamic policy */
void stop_policy_excl(void)
{
	release_dma_chan_excl();
	printk(KERN_DEBUG "ioat: DMA chan remove (exclusive chan policy)\n");
}
EXPORT_SYMBOL(stop_policy_excl);

/* start shared dynamic policy */
void start_policy_sh(void)
{
	copy_dmaengine_get();
	printk(KERN_DEBUG "ioat: DMA chan alloc (shared chan policy)\n");
}
EXPORT_SYMBOL(start_policy_sh);

/* stop shared dynamic policy */
void stop_policy_sh(void)
{
	copy_dmaengine_put();
	printk(KERN_DEBUG "ioat: DMA chan remove (shared chan policy)\n");
}
EXPORT_SYMBOL(stop_policy_sh);

static int __dma_copy_from_user(struct request_copy *req_cpy_local)
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

/** 
 * copy_from_user with DMA engine without pin/unpin (dynamic shared policy)
 * return: 0 for sucessfull, negative for error  
 * and positive for byte again to copy 
 * */ 
 int _dma_copy_from_user_sh(struct request_copy *req_cpy_local)
{	
	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_sh();	
	
		if (req_cpy_local->chan == NULL )
		{
			return -EBUSY;
		}
	
		return __dma_copy_from_user(req_cpy_local);
	}
	
	return 0;
}
EXPORT_SYMBOL(_dma_copy_from_user_sh);

/** 
 * copy_from_user with DMA engine without pin/unpin (dynamic exclusive policy)
 * return: 0 for sucessfull, negative for error  
 * and positive for byte again to copy 
 * */ 
 int _dma_copy_from_user_excl(struct request_copy *req_cpy_local)
{	
	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_excl();	
	
		if (req_cpy_local->chan == NULL )
		{
			return -EBUSY;
		}
	
		return __dma_copy_from_user(req_cpy_local);
	}
	
	return 0;
}
EXPORT_SYMBOL(_dma_copy_from_user_excl);
 
static int __dma_copy_to_user(struct request_copy *req_cpy_local)
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

/** 
 * copy_to_user with DMA engine without pin/unpin (dynamic shared policy)
 * return: 0 for sucessfull, negative for error 
 * and positive for byte again to copy
 */ 
int _dma_copy_to_user_sh(struct request_copy *req_cpy_local)
{	
	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_sh();	
		
		if (req_cpy_local->chan == NULL )
			return -EBUSY;

		return __dma_copy_to_user(req_cpy_local);
	}
	
	return 0;
}
EXPORT_SYMBOL(_dma_copy_to_user_sh);

/** 
 * copy_to_user with DMA engine without pin/unpin (dynamic exclusive policy)
 * return: 0 for sucessfull, negative for error 
 * and positive for byte again to copy
 */ 
int _dma_copy_to_user_excl(struct request_copy *req_cpy_local)
{	
	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_excl();	
		
		if (req_cpy_local->chan == NULL )
			return -EBUSY;

		return __dma_copy_to_user(req_cpy_local);
	}
	
	return 0;
}
EXPORT_SYMBOL(_dma_copy_to_user_excl);

/*
 * Pin down all the user pages needed for req_cp->len bytes.
*/
int dma_pin_user_pages(struct request_copy *req_cp , int write , int force)
{
	int ret;

	/* don't pin down non-user-address */
	if (segment_eq(get_fs(), KERNEL_DS))
		return PIN_NO_USER;

	/* determine how many pages there are, up front */
	req_cp->nr_pages = ((PAGE_ALIGN((unsigned long)req_cp->user_base + req_cp->len) - 
		((unsigned long)req_cp->user_base  & PAGE_MASK)) >> PAGE_SHIFT);

	/* single kmalloc for pinned list, page_list[], and the page arrays */
	req_cp->pages = kmalloc((sizeof(struct page*) * req_cp->nr_pages), GFP_KERNEL);
	if (!req_cp->pages)
		goto out;

	if (!access_ok(VERIFY_WRITE, req_cp->user_base, req_cp->len) && write)
		goto unpin;
	
	if (!access_ok(VERIFY_READ, req_cp->user_base, req_cp->len) && !write)
		goto unpin;

	/* pin pages down */
	down_read(&current->mm->mmap_sem);
	ret = get_user_pages( current, current->mm , (unsigned long) req_cp->user_base, req_cp->nr_pages,
			write ,	/* write */  force,	/* force */	req_cp->pages,	NULL);
	up_read(&current->mm->mmap_sem);

	if (ret != req_cp->nr_pages)
		goto unpin;

	return PIN_SUCCESSFUL;

unpin:
	dma_unpin_user_pages(req_cp , SET_DIRTY_OFF);
out:
	return PIN_FAILED;
}
EXPORT_SYMBOL(dma_pin_user_pages);

/*
 * Pin up all the user pages in the req_cp->pages.
*/
void dma_unpin_user_pages(struct request_copy *req_cp , int dirty)
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
EXPORT_SYMBOL(dma_unpin_user_pages);

/* 
 * copy memory in kernel space with DMA engine
 * return: 0 for sucessfull, check status for error 
 */ 
int dma_copy_from_kernel(struct request_copy *req_cpy_local)
{
	while(1)
	{
		req_cpy_local->dma_cookie = dma_async_memcpy_buf_to_buf(req_cpy_local->chan, 
									req_cpy_local->kernel_base, req_cpy_local->user_base, req_cpy_local->len);
									
		if(unlikely(req_cpy_local->dma_cookie < 0)) 
			dma_async_issue_pending(req_cpy_local->chan);	
		else
			goto dma_cookie_ok;	
	}		

dma_cookie_ok:
	req_cpy_local->status = dma_sync_wait(req_cpy_local->chan, req_cpy_local->dma_cookie);
	return 0;
}

/* 
 * copy memory in kernel space with DMA engine
 * return: 0 for sucessfull, check status for error 
 */ 
int dma_copy_to_kernel(struct request_copy *req_cpy_local)
{
	while(1)
	{
		req_cpy_local->dma_cookie = dma_async_memcpy_buf_to_buf(req_cpy_local->chan, 
									  req_cpy_local->user_base ,req_cpy_local->kernel_base, req_cpy_local->len);
									
		if(unlikely(req_cpy_local->dma_cookie < 0)) 
			dma_async_issue_pending(req_cpy_local->chan);	
		else
			goto dma_cookie_ok;
	}

dma_cookie_ok:
	req_cpy_local->status = dma_sync_wait(req_cpy_local->chan, req_cpy_local->dma_cookie);
	return 0;
}

/** 
 * copy_to_user with DMA engine (for copy in uaccess_64.h)
 * return: 0 for sucessfull , negative for error 
 * and positive for byte again to copy
 */ 
int dma_copy_to_user(struct request_copy *req_cpy_local)
{
	int ret = 0;
	
	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_boot();	
	
		if (req_cpy_local->chan == NULL )
			return -EBUSY;

		ret = dma_pin_user_pages(req_cpy_local , PIN_WRITE , 0) ;
		
		if( ret == PIN_FAILED)
			return req_cpy_local->len; 
	
		if( ret == PIN_NO_USER )
		{
			return dma_copy_to_kernel(req_cpy_local);
		}
		
		ret = __dma_copy_to_user(req_cpy_local);
		
		dma_unpin_user_pages(req_cpy_local , SET_DIRTY_ON);
	}
	
	return ret;
}
EXPORT_SYMBOL(dma_copy_to_user);

/** 
 * copy_from_user with DMA engine (for copy in uaccess_64.h)
 * return: 0 for sucessfull, negative for error  
 * and positive for byte again to copy 
 * */ 
 int dma_copy_from_user(struct request_copy *req_cpy_local)
{
	int ret = 0;
	
	if( req_cpy_local->len > 0)
	{	
		req_cpy_local->chan = copy_dma_get_channel_boot();	
	
		if (req_cpy_local->chan == NULL )
			return -EBUSY;

		ret = dma_pin_user_pages(req_cpy_local , PIN_READ , 0);
		
		if( ret == PIN_FAILED)
			return req_cpy_local->len; 
	
		if( ret == PIN_NO_USER )
			return dma_copy_from_kernel(req_cpy_local);
		
		ret = __dma_copy_from_user(req_cpy_local);
		
		dma_unpin_user_pages(req_cpy_local , SET_DIRTY_OFF);			
	}
	
	return ret;
}
EXPORT_SYMBOL(dma_copy_from_user);
