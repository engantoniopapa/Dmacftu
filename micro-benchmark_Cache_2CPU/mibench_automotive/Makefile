
mibench: qsort_small.c qsort_large.c susan.c main.c data_synthesis.c core_mibench.c
	gcc -Wall -static -O3 -o mibench susan.c main.c qsort_small.c qsort_large.c data_synthesis.c core_mibench.c -lm -lrt -pthread
clean:
	rm -rf mibench output*
