#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "You must be a root user" 2>&1
  exit 1
else
	rmmod micro-benchmark_Msg.ko
	
	rm -f /dev/micro-benchmark_from 
	rm -f /dev/micro-benchmark_to
	rm -f /dev/micro-benchmark_from_nc
	rm -f /dev/micro-benchmark_to_nc
fi
