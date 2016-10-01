all: daemon

daemon: daemon.o core_daemon.o daemon_operation.o
	gcc -Wall -o daemon daemon.o core_daemon.o daemon_operation.o

daemon.o: daemon.h core_daemon.h daemon_operation.h daemon.c
	gcc -Wall -c daemon.c

core_daemon.o: daemon.h core_daemon.h daemon_operation.h core_daemon.c
	gcc -Wall -c core_daemon.c

daemon_operation.o: daemon.h daemon_operation.h core_daemon.c
	gcc -Wall -c daemon_operation.c

clean:
	rm -rf *o daemon
