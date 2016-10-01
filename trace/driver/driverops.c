/* 
 * Driver Operation
*/

#include "trace_copy.h"

extern int count_region;

/* implemented for struct trace_data */
static int trace_driver_open(struct inode *inode, struct file *filp)
{	
	struct trace_dev *t_dev;
  
	t_dev = container_of(inode->i_cdev, struct trace_dev, c_dev);
	filp->private_data = (void *) &(t_dev->data);
  return 0;
}

static int trace_driver_close(struct inode *inode, struct file *filp)
{
  PRINT_DEBUG( "\nDriver: close()\n");
  return 0;
}

static loff_t trace_driver_lseek(struct file *filp, loff_t off, int whence)
{
  loff_t newpos;

  switch(whence) 
  {
    case 0: /* SEEK_SET */
      newpos = off;
      break;

    case 1: /* SEEK_CUR */
      newpos = filp->f_pos + off;
      break;

    case 2: /* SEEK_END not implemented set as SEEK_SET*/
      newpos = off;
      break;

    default: /* can't happen */
      return -EINVAL;
  }
  
  if (newpos < 0) return -EINVAL;
  filp->f_pos = newpos;
  return newpos;
}

/**
 * read @len measure for each cpu
 * @flip: for struct trace_data
 * @buf: user buffer to write the measures
 * @len: the number of measures to write for each cpu
 * @off: offset to @buf
 * Returns EFAULT if the measures could not be copied in the user space. 
 */
static ssize_t trace_driver_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	int j;
	struct trace_data *t_data;
	int ret;
	size_t size_reg;
	
	if(!buf)
		return -EINVAL;
  
	t_data = (struct trace_data *) filp->private_data;	
	
	if(len > BUFF_READ)
			len = BUFF_READ;
							
	for_each_present_cpu(j) 
	{	
		/* read max len measures for each cpu */			
		region_read_cpu(len , &per_cpu(*(t_data->ptr_cb) , j) , &(t_data->ptr_region[j]) );
	}
	
	size_reg = (sizeof(struct region_mem) * count_region);
	
	/* copy_to_user() not measured */
	ret = copy_to_user_pure(buf, t_data->ptr_region , size_reg);
	if(ret != 0 )
	 return -EIO;
	 
	return size_reg;
}


/* not implemented */ 
static ssize_t trace_driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	PRINT_DEBUG("write called\n");
	return -EPERM;
}

static struct file_operations pugs_fops =
{
  .owner = THIS_MODULE,
  .llseek = trace_driver_lseek,
  .open = trace_driver_open,
  .release = trace_driver_close,
  .read = trace_driver_read,
  .write = trace_driver_write,
};

/* removes all device */
void all_trace_device_destroy(struct trace_dev *t_dev, int size , struct class *trace_class)
{
	int i ;
	
	for( i = 0; i < size ; ++i)
			device_destroy(trace_class, t_dev[i].devt);	
}

/* removes all cdev */
void all_trace_driver_del_chardev(struct trace_dev *t_dev, int size)
{
	int i ;
	
	for( i = 0; i < size ; ++i)
			 cdev_del( &(t_dev[i].c_dev) );
}

/**
* init all cdev structure
* @t_dev: the array of struct trace_dev
* @size: the size of  @t_dev
* @driver_major: driver major number
* returns zero on success, or negative value for error.
*/
int trace_driver_add_chardev(struct trace_dev *t_dev, int size , s32 driver_major)
{
	int i ;
	int ret = 0;
	
	/* adds a char devices to the system */
  for( i = 0 ; i < size ;++i)
  {		
		t_dev[i].devt = MKDEV(driver_major, i);
		cdev_init(&(t_dev[i].c_dev), &pugs_fops);
		t_dev[i].c_dev.owner = THIS_MODULE;
		ret = cdev_add(&(t_dev[i].c_dev) , t_dev[i].devt , 1);
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
		cdev_del( &(t_dev[i].c_dev) );
		--i;
	}
	return ret;
}	

/**
* init all cdev structure
* @t_dev: the array of struct trace_dev
* @size: the size of  @t_dev
* @trace_class: driver class
* returns zero on success, or negative value for error.
*/
int trace_device_create(struct trace_dev *t_dev, int size , struct class *trace_class)
{
	int i;
	int ret = 0;
	struct device *tmp_device;
	
	/* creates the devices and registers them with sysfs */
  for( i = 0 ; i < size ;++i)
  {
		tmp_device = device_create(trace_class, NULL, t_dev[i].devt , NULL, t_dev[i].name) ;
		if(IS_ERR(tmp_device))
		{
			printk(KERN_WARNING "%s: Failed to create device: %s \n" ,DRIVER_NAME, t_dev[i].name );
			ret = PTR_ERR(tmp_device);
			--i;
			goto err_dev_create;
		}
	}

return ret;
	
err_dev_create:
	while (i >= 0)
	{
		device_destroy(trace_class, t_dev[i].devt);	
		--i;
	}
	return ret;
}
