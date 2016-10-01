#include "daemon.h"
#include "core_daemon.h"
#include "daemon_operation.h"

int main(int argc, char *argv[])
{
	char path_dev[SIZE_PATH]; /* buffer for path of device file */ 
	int n_cpu; /* number of CPU */
	unsigned long delay; /* delay  between read (in nsec) */
	unsigned long n_read; /* number of read() to device file */
	struct daemon_trace daemon; /* struct daemon_cpu vector */
	
	/* check on number of input parameter */
	if (argc != 5) 
	{
		printf("daemon <number of CPU> <duration of the measurement (in sec)> <delay between read (in nsec)> <name dev file> \n");
		return EXIT_FAILURE;
	}
	
	printf("\nStart %s for %s \n" , argv[0] , argv[4]);

	/* argv[1]: <number of CPU> */
	n_cpu = atoi(argv[1]);
	
	/* argv[3]: delay  between read (in nsec)*/
	delay = strtoul(argv[3],NULL,0);
			
	/* argv[4]: <name dev file> */
	memset(path_dev , '\0' , SIZE_PATH );
	sprintf(path_dev ,"%s%s" ,DIR_DEV, argv[4]);
	
	n_read = strtoul(argv[2],NULL,0) * (SEC_IN_MCS/ delay);
	
	/* init a stuct deamon_cpu for each cpu */
	init_daemon(&daemon , n_cpu , n_read , argv[4]);
	
	/* save the measures in the regular file */
	if(record_measures(&daemon , n_cpu , n_read , delay , path_dev) < 0)
		return EXIT_FAILURE;
		
	printf("\nEnd %s for %s \n" , argv[0] , argv[4]);
	return EXIT_SUCCESS;
}
