#ifndef _TRACE_COPY_H_
#define _TRACE_COPY_H_

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
#include <linux/cpumask.h>
#include <linux/string.h>
#include <linux/percpu.h>
#include <linux/circular_buffer.h>

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif

#ifndef TRACE_COPY_NAME
#define TRACE_COPY_NAME    "mem_trace"
#endif

#ifndef TRACE_COPY_VERSION
#define TRACE_COPY_VERSION  "0.4"
#endif

#ifdef MY_DEBUG
#      define PRINT_DEBUG(fmt, ...)  \
              printk(KERN_DEBUG  pr_fmt(fmt), ##__VA_ARGS__)
#else 
#      define PRINT_DEBUG(format_string, args...)
#endif


#define BUFF_READ 500 /* unit buffer to read*/
#define TRACE_MINOR 4 /* cardinality of the minor numbers */
#define MAX_CHAR_NAME 128 /* max char for name trace */

/**
 * structure to record a measure
 * @index_cpu_event: index of the cpu used to increase the counter event 
 * @count event: value of count event for measure
 * @addr: return address of copy_to/from_user
 * @size: copied data (byte) 
 * @type_op: the hardware type that runs operation
 * @index_cpu: cpu that finishes the measure
 * @clock_cpu_start: start measure time (ns) 
 * @clock_cpu_end: end measure time (ns) 
*/ 
struct unit_buff_user
{
	unsigned short index_cpu_event; /*indice cpu riferita al contatore evento*/
	long long count_event; /* contatore evento*/
  void *addr; /*indirizzo di ritorno della funzione instrumentata*/
  unsigned long size; /*size della memoria copiata*/
  unsigned short type_op; /*se l'operazione è stata eseguita con il dma o con la cpu*/
  unsigned short index_cpu; /* indice cpu riferita alla copy_..() */
  unsigned long long clock_cpu_start; /* tempo in nanosecondi di inizio misura */
  unsigned long long clock_cpu_end; /* tempo in nanosecondi di fine misura */
};

/**
 * structure to group measures, one for each cpu
 * @count: number of the measures
 * @buffer: vector of the measures
 */
struct region_mem
{
	unsigned int count; /* quante misurazioni contiene la region */
	struct unit_buff_user buffer[BUFF_READ]; /* array di misuarzione */
};

/**
 * data struct, one for each device
 * @ptr_region: structure to group measures, one for each cpu
 * @ptr_cb: struct kernel for trace
 */
struct trace_data
{
	struct region_mem *ptr_region;
	struct CircularBuffer *ptr_cb;
};

/**
 * main device descriptor, one for each device
 * @data: data struct
 * @name: name of device
 * @devt: device number (major,minor)
 * @c_dev: struct cdev for the container_of()
 */
struct trace_dev
{
	struct trace_data data;
	char name[MAX_CHAR_NAME];
	dev_t devt;
	struct cdev c_dev;
};


#include "driverops.h"
#include "core_trace_copy.h"

#endif

