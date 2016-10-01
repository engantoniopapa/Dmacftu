#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>

/* hardware operation */
#define OP_COPY_NO_DEF 0 
#define OP_COPY_CPU 1
#define OP_COPY_DMA 2

#define CIRC_BUFF_SIZE 1024 


/** struct unit_buff - struct to record the measurement
 * @index_cpu_event: cpu that starts the measure 
 * @count_event: value of counter on that cpu (index_cpu_event)
 * @addr: return address of the current function
 * @size: copied data (byte) from the operation 
 * @type_op: the hardware type that runs operation
 * @index_cpu: cpu that finishes the measure
 * @clock_cpu_start: start measure time (ns) 
 * @clock_cpu_end: end measure time (ns) 
 */ 
struct unit_buff
{
  unsigned short index_cpu_event;
  unsigned long long count_event; 
  void *addr;
  unsigned long size;
  unsigned short type_op;
  unsigned short index_cpu;
  unsigned long long clock_cpu_start;
  unsigned long long clock_cpu_end;
};
 
/** Circular buffer struct - object for the measurement on the cpu
 * @ count_event: counter event on that cpu
 * @ start: index of oldest element
 * @ end: index at which to write new element
 * @ size: size of buffer
 * @ array_element: set of element
 */
struct CircularBuffer
{
	long long count_event; /* contatore evento*/
    uint         start;  /*               */
    uint         end;    /*   */
    uint         size;
	struct unit_buff *array_element; /* vector of elements         */
};

/* alloc and init an instance of struct CircularBuffer */
void cbInit(struct CircularBuffer *cb );

/* check if is full, return: 1 for full, 0 for not full */
static inline int cbIsFull(struct CircularBuffer *cb) 
{
    return (cb->end + 1) % cb->size == cb->start;     
}
 
/* check if is empty return: 1 for empty, 0 for not empty */
static inline int cbIsEmpty(struct CircularBuffer *cb) 
{
    return cb->end == cb->start; 
}
 
/* increment counter and return its value */
static inline long long get_counter(struct CircularBuffer *cb) 
{
  return ++cb->count_event;
}

/* free an instance of struct CircularBuffer */
void cbFree(struct CircularBuffer *cb);
 
/* Write an element, not overwriting oldest element if buffer is full (wine policy).  */
void cbWrite(struct CircularBuffer *cb, struct unit_buff *elem); 

/* Read oldest element and return a its copy. App must ensure !cbIsEmpty() first. */
void cbRead(struct CircularBuffer *cb, struct unit_buff *elem);

/* copy at most @size oldest element and returns the number of elements copied */
int cbRead_list(struct CircularBuffer *cb, struct unit_buff *elem , int size);

#endif /*CIRCULAR_BUFFER_H*/
