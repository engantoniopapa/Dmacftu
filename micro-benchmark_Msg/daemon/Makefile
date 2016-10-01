all: daemon

daemon: daemon.o core_daemon.o data_synthesis.o
	gcc -Wall -o daemon daemon.o core_daemon.o data_synthesis.o -lm

daemon.o: daemon.h core_daemon.h daemon.c
	gcc -Wall -c daemon.c

core_daemon.o: daemon.h core_daemon.h core_daemon.c
	gcc -Wall -c core_daemon.c
	
data_synthesis.o:  daemon.h core_daemon.h data_synthesis.c
	gcc -Wall -c data_synthesis.c

clean:
	rm -rf *o daemon
