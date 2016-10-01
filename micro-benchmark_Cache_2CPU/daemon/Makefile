all: daemon

daemon: daemon.o core_daemon.o
	gcc -pthread -Wall -o daemon daemon.o core_daemon.o 

daemon.o: daemon.h core_daemon.h daemon.c
	gcc -Wall -c daemon.c

core_daemon.o: daemon.h core_daemon.h core_daemon.c
	gcc -Wall -c core_daemon.c

clean:
	rm -rf *o daemon
