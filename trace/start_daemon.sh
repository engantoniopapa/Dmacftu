#!/bin/bash

#$1: numero cpu attive
#$2: quanti secondi di vita hanno i demoni
#$3: quanti microsecondi dura il delay tra due read() al driver
 make clean -C daemon/
 make  -C daemon/
 
 cd daemon
 ./daemon $1 $2 $3	mem_trace_from &
 ./daemon $1 $2 $3	mem_trace_to &
 ./daemon $1 $2 $3	__mem_trace_from &
 ./daemon $1 $2 $3	__mem_trace_to &

	
 cd ..
