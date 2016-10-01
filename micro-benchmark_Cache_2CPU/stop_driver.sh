#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "You must be a root user" 2>&1
  exit 1
else
	rmmod micro-benchmark_Cache_2CPU.ko
	
	rm -f /dev/micro-benchmark_ch0
	rm -f /dev/micro-benchmark_ch1
	rm -f /dev/micro-benchmark_ch2
	rm -f /dev/micro-benchmark_ch3
fi
