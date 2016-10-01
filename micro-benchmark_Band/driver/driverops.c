/* 
 * Driver Operation
*/

#include "micro_benchmark.h"

extern short int type_measure;

/* implemented for struct benchmark_data */
static int benchmark_driver_open(struct inode *inode, struct file *filp)
{	
	struct benchmark_dev *b_dev;
  
	b_dev = container_of(inode->i_cdev, struct benchmark_dev, c_dev);
	filp->private_data = (void *) b_dev->data;
  return 0;
}

static int benchmark_driver_close(struct inode *inode, struct file *filp)
{
  PRINT_DEBUG( "\nDriver: close()\n");
  return 0;
}

/**
 * send measure result to user 
 * @flip: for struct benchmark_data
 * @buf: user buffer for test
 * @len: size of @buf 
 * @off: not use
 * Returns negative number for error. On success, returns the number of bytes written to the file.
 */
static ssize_t benchmark_driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	struct benchmark_data *b_data;
	int ret = -EPERM; 
	
	*off = 0;
	
	if(!buf)
		return -EINVAL;
		
	b_data = (struct benchmark_data *) filp->private_data;	
	
	if( len > b_data->kernel_size)
		return -EFAULT;		

	switch (b_data->type) 
	{
	
		case _CPU_MEASURE:
		{
			ret = copy_from_user_pure(b_data->kernel_buf, b_data->user_buf, b_data->user_size);
		}
		break;
	
		case _EXCLUSIVE_MEASURE:
		{
			ret = _dma_copy_from_user_excl(&(b_data->req_cpy_local));
		}
		break;
	
		case _SHARED_MEASURE:
		{
			ret = _dma_copy_from_user_sh(&(b_data->req_cpy_local));
		}
		break;
		
		case _PRIORITY_MEASURE:
		{
				ret = _dma_copy_from_user_prio(&(b_data->req_cpy_local_prio));
		}
		break;
	}

	if(ret < 0)
		return ret;

	return (len - ret); 
}

/**
 * send measure result to user 
 * @flip: for struct benchmark_data
 * @buf: user buffer to write the measures
 * @len: the number of measures 
 * @off: not use
 * Returns negative number for error. On success, returns the number of bytes read.
 */
static ssize_t benchmark_driver_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	struct benchmark_data *b_data;
	int ret = -EPERM; 

  *off = 0;
  	
	if(!buf)
		return -EINVAL;
		
	b_data = (struct benchmark_data *) filp->private_data;	

	if( len > b_data->kernel_size)
	return -EFAULT;		

	switch (b_data->type) 
	{
	
		case _CPU_MEASURE:
		{
			ret = copy_to_user_pure(b_data->user_buf, b_data->kernel_buf, b_data->user_size);
		}
		break;
	
		case _EXCLUSIVE_MEASURE:
		{
			ret = _dma_copy_to_user_excl(&(b_data->req_cpy_local));
		}
		break;
	
		case _SHARED_MEASURE:
		{
			ret = _dma_copy_to_user_sh(&(b_data->req_cpy_local));
		}
		break;
		
		case _PRIORITY_MEASURE:
		{
				ret = _dma_copy_to_user_prio(&(b_data->req_cpy_local_prio));
		}
		break;
	}
	
	if(ret < 0)
		return ret;
	 
	return (len - ret);
}

static long benchmark_driver_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	query_arg_t q;
	struct benchmark_data *b_data;
	int ret;
	
	b_data = (struct benchmark_data *) filp->private_data;	

	switch (cmd)
	{
		case MB_LOAD:
		{
			if (copy_from_user_pure(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
			{
					return -EACCES;
			}
			
			b_data->user_size = q.len;
			b_data->user_buf = (char *) q.buf_user;
			get_random_bytes(b_data->kernel_buf, b_data->user_size);

			if(type_measure == _EXCLUSIVE_MEASURE || type_measure == _SHARED_MEASURE)
			{
				init_req_copy(&(b_data->req_cpy_local) , (void *) b_data->user_buf, 
						(void *) b_data->kernel_buf, b_data->user_size);

				ret = dma_pin_user_pages(&(b_data->req_cpy_local) , q.write , 0) ;

				if(ret != PIN_SUCCESSFUL)
				{
					printk(KERN_WARNING "%s: Error Pin user pages\n" , DRIVER_NAME);
					return -EACCES;
				}
			}	
	
			if(type_measure == _PRIORITY_MEASURE)
			{
				init_req_copy_prio(&(b_data->req_cpy_local_prio) , (void *) b_data->user_buf, 
					(void *) b_data->kernel_buf, b_data->user_size , b_data->id_chan);

				ret = dma_pin_user_pages_prio(&(b_data->req_cpy_local_prio) , q.write , 0);

				if(ret != PIN_SUCCESSFUL)
				{
					printk(KERN_WARNING "%s: Error Pin user pages\n" , DRIVER_NAME);
					return -EACCES;
				}
			}	
		}
		break;
		
		case MB_UNLOAD:
		{
			if (copy_from_user_pure(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
			{
					return -EACCES;
			}
			
			/* unpin user pages for DMA */
			if(type_measure == _EXCLUSIVE_MEASURE || type_measure == _SHARED_MEASURE)
			{
				if(q.write == 0)
					dma_unpin_user_pages(&(b_data->req_cpy_local) , SET_DIRTY_OFF);
				else
					dma_unpin_user_pages(&(b_data->req_cpy_local) , SET_DIRTY_ON);
			}	

			if(type_measure == _PRIORITY_MEASURE)
			{
				if(q.write == 0)
					dma_unpin_user_pages_prio(&(b_data->req_cpy_local_prio) , SET_DIRTY_OFF);
				else
					dma_unpin_user_pages_prio(&(b_data->req_cpy_local_prio) , SET_DIRTY_ON);
			}	
		}
		break;
				
		default:
				return -EINVAL;
	}

	return 0;
}

static struct file_operations pugs_fops =
{
  .owner = THIS_MODULE,
  .open = benchmark_driver_open,
  .release = benchmark_driver_close,
  .read = benchmark_driver_read,
  .write = benchmark_driver_write,
  .unlocked_ioctl = benchmark_driver_ioctl,
};

/* removes all device */
void all_mb_device_destroy(struct benchmark_dev *b_dev, int size , struct class *mb_class)
{
	int i ;
	
	for( i = 0; i < size ; ++i)
			device_destroy(mb_class, b_dev[i].devt);	
}

/* removes all cdev */
void all_mb_driver_del_chardev(struct benchmark_dev *b_dev, int size)
{
	int i ;
	
	for( i = 0; i < size ; ++i)
			 cdev_del( &(b_dev[i].c_dev) );
}

/**
* init all cdev structure
* @b_dev: the array of struct benchmark_dev
* @size: the size of  @b_dev
* @driver_major: driver major number
* returns zero on success, or negative value for error.
*/
int mb_driver_add_chardev(struct benchmark_dev *b_dev, int size, 
		s32 driver_major)
{
	int i ;
	int ret = 0;
	
	/* adds a char devices to the system */
  for( i = 0 ; i < size ;++i)
  {
		b_dev[i].devt = MKDEV(driver_major, i);
		cdev_init(&(b_dev[i].c_dev), &pugs_fops);
		b_dev[i].c_dev.owner = THIS_MODULE;
		ret = cdev_add(&(b_dev[i].c_dev) , b_dev[i].devt , 1);
		if(ret <  0)
		{
			printk(KERN_WARNING "%s: Failed to cdev_add() \n" , DRIVER_NAME);
			--i;
			goto err_cdev;
		}
	}
	 
	return ret;
err_cdev:
	while (i >= 0)
	{
		cdev_del( &(b_dev[i].c_dev) );
		--i;
	}
	return ret;
}	

/**
* init all cdev structure
* @b_dev: the array of struct benchmark_dev
* @size: the size of  @b_dev
* @mb_class: driver class
* returns zero on success, or negative value for error.
*/
int mb_device_create(struct benchmark_dev *b_dev, int size, struct class *mb_class)
{
	int i;
	int ret = 0;
	struct device *tmp_device;
	
	/* creates the devices and registers them with sysfs */
  for( i = 0 ; i < size ;++i)
  {
		tmp_device = device_create(mb_class, NULL, b_dev[i].devt , NULL, b_dev[i].name) ;
		if(IS_ERR(tmp_device))
		{
			printk(KERN_WARNING "%s: Failed to create device: %s \n" ,DRIVER_NAME, b_dev[i].name );
			ret = PTR_ERR(tmp_device);
			--i;
			goto err_dev_create;
		}
	}

return ret;
	
err_dev_create:
	while (i >= 0)
	{
		device_destroy(mb_class, b_dev[i].devt);	
		--i;
	}
	return ret;
}

