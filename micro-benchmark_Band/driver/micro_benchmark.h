#ifndef _MICRO_BENCHMARK_H_
#define _MICRO_BENCHMARK_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>
#include <linux/dmaengine.h>
#include "ioctl_mb.h"

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif

#ifndef BENCHMARK_NAME
#define BENCHMARK_NAME     "micro-benchmark_Band"
#endif

#ifndef BENCHMARK_VERSION
#define BENCHMARK_VERSION  "0.3"
#endif

#ifdef MY_DEBUG
#      define PRINT_DEBUG(fmt, ...)  \
              printk(KERN_DEBUG  pr_fmt(fmt), ##__VA_ARGS__)
#else 
#      define PRINT_DEBUG(format_string, args...)
#endif

#define MAX_CHAR_NAME 128 /* max char for name benchmark */
#define MB_MINOR 4 /* cardinality of the minor numbers */

/* type measures */
#define _CPU_MEASURE 1
#define _EXCLUSIVE_MEASURE 2
#define _SHARED_MEASURE 3
#define _PRIORITY_MEASURE 4

/* time in ns for DMA priority policy */
#define _NANOSEC_HIGH 30000
#define _NANOSEC_LOW  60000

/**
 * structure to group measures, one for each device
 * @user_size: memory size of @user_buf
 * @user_buf: buffer in user space for measure
 * @kernel_size: memory size of @kernel_buf
 * @kernel_buf: buffer in kernel space for measure
 * @id_chan: id for DMA channel
 * @request_copy: for DMA operation 
 * @request_copy_prio: for DMA priority operation 
 * @type: type of operation to measure
 */
struct benchmark_data
{
	unsigned long user_size;
	char * __user user_buf;
	unsigned long kernel_size;
	char * __kernel kernel_buf;
	short int id_chan;
	struct request_copy req_cpy_local;
	struct request_copy_prio req_cpy_local_prio;
	unsigned short int type;
};

/**
 * main device descriptor, one for each device
 * @data: data struct
 * @name: name of device
 * @devt: device number (major,minor)
 * @c_dev: struct cdev for the container_of()
 */
struct benchmark_dev
{
	struct benchmark_data *data;
	char name[MAX_CHAR_NAME];
	dev_t devt;
	struct cdev c_dev;
};

#include "driverops.h"
#include "core_benchmark.h"

#endif

