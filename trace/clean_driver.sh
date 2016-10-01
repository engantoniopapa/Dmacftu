#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "You must be a root user" 2>&1
  exit 1
else
	make clean -C driver/

	
	rm -f /dev/mem_trace_to
	rm -f /dev/mem_trace_from
	rm -f /dev/__mem_trace_to
	rm -f /dev/__mem_trace_from 
fi
