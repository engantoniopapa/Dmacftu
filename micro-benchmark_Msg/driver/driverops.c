/* 
 * Driver Operation
*/

#include "micro_benchmark.h"


/* implemented for struct benchmark_data */
static int benchmark_driver_open(struct inode *inode, struct file *filp)
{	
	struct benchmark_dev *b_dev;
  
	b_dev = container_of(inode->i_cdev, struct benchmark_dev, c_dev);
	filp->private_data = (void *) &(b_dev->data);
  return 0;
}

static int benchmark_driver_close(struct inode *inode, struct file *filp)
{
  PRINT_DEBUG( "\nDriver: close()\n");
  return 0;
}

static loff_t benchmark_driver_lseek(struct file *filp, loff_t off, int whence)
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
 * send measure result to user 
 * @flip: for struct benchmark_data
 * @buf: user buffer for test
 * @len: size of @buf 
 * @off: not use
 * Returns EFAULT if the measures could not be copied in the user space. 
 * On success, this will be zero.
 */
static ssize_t benchmark_driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	struct benchmark_data *b_data;
	int ret;
	
	if(!buf)
		return -EINVAL;
	
	b_data = (struct benchmark_data *) filp->private_data;	
	
	if( len > b_data->kernel_size)
		return -EFAULT;		

	b_data->user_size = len;
	b_data->user_buf = (char *) buf;
	get_random_bytes(b_data->kernel_buf, b_data->user_size);

	ret = init_measure(b_data);
	
	if(ret < 0)
		return -EFAULT;		
  
  return len;
}

/**
 * send measure result to user 
 * @flip: for struct benchmark_data
 * @buf: user buffer to write the measures
 * @len: the number of measures 
 * @off: not use
 * Returns EFAULT if the measures could not be copied in the user space. 
 * On success, this will be zero.
 */
static ssize_t benchmark_driver_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	struct benchmark_data *b_data;
	int ret;
  
  if(!buf)
		return -EINVAL;
		
	b_data = (struct benchmark_data *) filp->private_data;	

	ret = copy_to_user_pure(buf, b_data->results , sizeof(struct benchmark_output) );

	if(ret != 0 )
	 return -EFAULT;
	 
	return sizeof(struct benchmark_output);
}

static struct file_operations pugs_fops =
{
  .owner = THIS_MODULE,
  .llseek = benchmark_driver_lseek,
  .open = benchmark_driver_open,
  .release = benchmark_driver_close,
  .read = benchmark_driver_read,
  .write = benchmark_driver_write,
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
int mb_driver_add_chardev(struct benchmark_dev *b_dev, int size,	s32 driver_major)
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
int mb_device_create(struct benchmark_dev *b_dev, int size , struct class *mb_class)
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

