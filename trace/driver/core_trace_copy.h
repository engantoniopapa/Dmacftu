#ifndef _CORE_TRACE_COPY_H_
#define _CORE_TRACE_COPY_H_

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif

void clean_trace_region_mem(struct trace_dev *t_dev);

int init_trace_dev(struct trace_dev *t_dev ,struct CircularBuffer *cb , char *name);

void region_read_cpu(int len , struct CircularBuffer *ptr_cb , struct region_mem *reg_m);

#endif
