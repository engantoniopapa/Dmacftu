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
#define BENCHMARK_NAME     "micro-benchmark_Msg"
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

#define BUFF_MEASURE 300 /* unit buffer for measure*/
#define MAX_CHAR_NAME 128 /* max char for name benchmark */
#define MB_MINOR 4 /* cardinality of the minor numbers */

#define ID_NO_OP 0 /* id operation for test (no operation) */
#define ID_COPY_FROM 1 /* id operation for test (copy from user) */
#define ID_COPY_TO 2 /* id operation for test (copy to user) */
#define ID_COPY_FROM_NC 3 /* id operation for test  (copy from user, no cache)*/
#define ID_COPY_TO_NC 4 /* id operation for test (copy to user, no cache) */

/* type measures */
#define _CPU_MEASURE 0
#define _EXCLUSIVE_MEASURE 1
#define _SHARED_MEASURE 2


/**
 * structure to record a measure
 * @index_cpu: index of the cpu for measure
 * @size: copied data (byte) 
 * @clock_cpu_start: start measure time (ns) 
 * @clock_cpu_end: end measure time (ns) 
*/ 
struct unit_buff_user
{
	unsigned short index_cpu; 
  unsigned long size; 
  unsigned long long clock_cpu_start; 
  unsigned long long clock_cpu_end; 
};

/**
 * micro-benchmark results
 * @count: number of the measures
 * @buffer: vector of the measures
 */
struct benchmark_output
{
	unsigned int count; 
	struct unit_buff_user buffer[BUFF_MEASURE];
};

/**
 * structure to group measures, one for each device
 * @user_size: memory size of @user_buf
 * @user_buf: buffer in user space for measure
 * @kernel_size: memory size of @kernel_buf
 * @kernel_buf: buffer in kernel space for measure
 * @type: type of operation to measure
 * @results: for benchmark results
 */
struct benchmark_data
{
	unsigned long user_size;
	char * __user user_buf;
	unsigned long kernel_size;
	char * __kernel kernel_buf;
	unsigned short int type;
	struct benchmark_output *results;
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
	struct benchmark_data data;
	char name[MAX_CHAR_NAME];
	dev_t devt;
	struct cdev c_dev;
};

#include "driverops.h"
#include "core_benchmark.h"

#endif

