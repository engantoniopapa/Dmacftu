#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "You must be a root user" 2>&1
  exit 1
else
#start module
 make clean -C driver/
 make  -C driver/ 
 echo "Micro-Benchmark on: "
 echo "0: CPU"
 echo "1: DMA (exclusive policy)"
 echo "2: DMA (shared policy)"
 echo -n ": "
 read type
 insmod driver/micro-benchmark_Msg.ko type_measure=${type}


#create device file
 device_f=/dev/micro-benchmark_from
 path_sys=/sys/class/micro-benchmark_Msg/micro-benchmark_from/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
 
 device_f=/dev/micro-benchmark_to
 path_sys=/sys/class/micro-benchmark_Msg/micro-benchmark_to/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
 
 device_f=/dev/micro-benchmark_from_nc
 path_sys=/sys/class/micro-benchmark_Msg/micro-benchmark_from_nc/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
 
 device_f=/dev/micro-benchmark_to_nc
 path_sys=/sys/class/micro-benchmark_Msg/micro-benchmark_to_nc/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
fi
