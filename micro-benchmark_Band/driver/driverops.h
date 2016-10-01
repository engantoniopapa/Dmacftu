#ifndef _DRIVEROPS_H_
#define _DRIVEROPS_H_

#ifndef __KERNEL__
#error UserMode programs should not include this file
#endif

void all_mb_device_destroy(struct benchmark_dev *b_dev, int size , struct class *mb_class);

void all_mb_driver_del_chardev(struct benchmark_dev *b_dev, int size);

int mb_driver_add_chardev(struct benchmark_dev *b_dev, int size, s32 driver_major);

int mb_device_create(struct benchmark_dev *b_dev, int size , struct class *mb_class);


#endif
