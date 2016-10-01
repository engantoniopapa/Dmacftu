#ifndef _DRIVEROPS_H_
#define _DRIVEROPS_H_

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif

void all_trace_device_destroy(struct trace_dev *t_dev, int size , struct class *trace_class);

void all_trace_driver_del_chardev(struct trace_dev *t_dev, int size);

int trace_driver_add_chardev(struct trace_dev *t_dev, int size , s32 driver_major);

int trace_device_create(struct trace_dev *t_dev, int size , struct class *trace_class);


#endif
