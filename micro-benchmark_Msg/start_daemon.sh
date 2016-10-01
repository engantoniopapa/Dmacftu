#!/bin/bash

 make clean -C daemon/
 make  -C daemon/
 
 cd daemon
 
 echo "Micro-Benchmark size test: "
 echo "1)  2 MB"
 echo "2)  4 MB"
 echo "3)  8 MB"
 echo "4)  16 MB"
 echo "5)  32 MB"
 echo "6)  64 MB"
 echo "7)  128 MB"
 echo "8)  256 MB"
 echo "9)  512 MB"
 echo "10) 1024 MB"
 echo -n ": "
 read size
 
 ./daemon micro-benchmark_from ${size} 	
 ./daemon micro-benchmark_to ${size}	
 ./daemon micro-benchmark_from_nc ${size} 
 ./daemon micro-benchmark_to_nc ${size} 

 cd ..

 echo ".... End Micro-benchmark"
