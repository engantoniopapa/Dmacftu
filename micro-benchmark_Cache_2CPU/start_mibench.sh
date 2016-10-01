#!/bin/bash

 make clean -C mibench_automotive/
 make  -C mibench_automotive/
 
 cd mibench_automotive
 
 echo ""
 echo " - MiBench Test -"
 echo "insert #test"
 echo -n ": "
 read n_test
 
 echo "name file:"
 echo -n ": "
 read name
 
 ./mibench ${n_test} ${name}

 cd ..

 echo ".... End MiBench"
