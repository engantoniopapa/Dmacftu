#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "You must be a root user" 2>&1
  exit 1
else
#start module
 make clean -C driver/
 make  -C driver/ 
 echo "Micro-Benchmark on: "
 echo "1: CPU"
 echo "2: DMA (exclusive policy)"
 echo "3: DMA (shared policy)"
 echo "4: DMA (priority policy)"
 echo -n ": "
 read type
 insmod driver/micro-benchmark_Band.ko type_measure=${type}


#create device file
 device_f=/dev/micro-benchmark_ch0
 path_sys=/sys/class/micro-benchmark_Band/micro-benchmark_ch0/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
 
 device_f=/dev/micro-benchmark_ch1
 path_sys=/sys/class/micro-benchmark_Band/micro-benchmark_ch1/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
 
 device_f=/dev/micro-benchmark_ch2
 path_sys=/sys/class/micro-benchmark_Band/micro-benchmark_ch2/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
 
 device_f=/dev/micro-benchmark_ch3
 path_sys=/sys/class/micro-benchmark_Band/micro-benchmark_ch3/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
fi
