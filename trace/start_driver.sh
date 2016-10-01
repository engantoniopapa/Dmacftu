#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "You must be a root user" 2>&1
  exit 1
else
#start module
 make clean -C driver/
 make  -C driver/ 
 insmod driver/mem_trace.ko 


#create device file
 device_f=/dev/mem_trace_to
 path_sys=/sys/class/mem_trace/mem_trace_to/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}

 device_f=/dev/mem_trace_from
 path_sys=/sys/class/mem_trace/mem_trace_from/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
 
 device_f=/dev/__mem_trace_to
 path_sys=/sys/class/mem_trace/__mem_trace_to/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}

 device_f=/dev/__mem_trace_from
 path_sys=/sys/class/mem_trace/__mem_trace_from/dev
 DEV_M=`cat ${path_sys}`
 DEV_M=${DEV_M/:/ }
 rm -f ${device_f}
 mknod ${device_f} c ${DEV_M}
 chmod 0666 ${device_f}
fi
