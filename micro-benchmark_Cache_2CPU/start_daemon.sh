#!/bin/bash

 make clean -C daemon/
 make  -C daemon/
 
 cd daemon
 
 echo ""
 echo " - Micro-Benchmark-Cache 2CPU-"
 echo "Size buffer memory in byte:"
 echo -n ": "
 read size
 
 echo "insert  type_operation:"
 echo "1) copy_from_user()"
 echo "2) copy_to_user()"
 echo "3) copy_from_user() + copy_to_user()"
 echo -n ": "
 read type_op
 
 ./daemon ${size} ${type_op}

 cd ..

 echo ".... End Micro-benchmark"
