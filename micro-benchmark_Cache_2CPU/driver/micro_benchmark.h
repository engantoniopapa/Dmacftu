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

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif

#ifndef BENCHMARK_NAME
#define BENCHMARK_NAME     "micro-benchmark_Cache_2CPU"
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
#define _16MB_TO_BYTE 16777216 /* 16MB in byte */ 

/* type measures */
#define _CPU_MEASURE 1
#define _DMA_MEASURE 2

/**
 * structure to group measures, one for each device
 * @kernel_size: memory size of @kernel_buf
 * @kernel_buf: buffer in kernel space for measure
 * @counter_op_read: number of read()
 * @counter_op_write: number of write()
 * @type: type of operation to measure
 */
struct benchmark_data
{
	unsigned long kernel_size;
	char * __kernel kernel_buf;
	unsigned long long counter_op_read;
	unsigned long long counter_op_write;
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

