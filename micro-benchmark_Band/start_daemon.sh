#!/bin/bash

 make clean -C daemon/
 make  -C daemon/
 
 cd daemon
 
 echo ""
 echo " - Micro-Benchmark-Bandwith -"
 echo "Size max:"
 echo "1)  2 MB"
 echo "2)  4 MB"
 echo "3)  8 MB"
 echo "4)  16 MB"
 echo "5)  32 MB"
 echo "6)  64 MB"
 echo "7)  128 MB"
 echo "8)  256 MB"
 echo -n ": "
 read size
 
 echo "insert #operation:"
 read n_op
 
 ./daemon ${size} ${n_op}

 cd ..

 echo ".... End Micro-benchmark"
