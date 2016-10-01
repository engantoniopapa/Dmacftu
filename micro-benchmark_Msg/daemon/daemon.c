#include "daemon.h"
#include "core_daemon.h"

int main(int argc, char *argv[])
{
	char path_dev[SIZE_PATH]; /* buffer for path of device file */ 
	unsigned long max_size; /* max size byte */
	struct daemon_benchmark *daemon_b; /* struct daemon */
	
	/* check on number of input parameter */
	if (argc != 3) 
	{
		printf("daemon <name dev file> <max byte message>\n");
		return EXIT_FAILURE;
	}
	
	printf("\nStart %s for %s \n" , argv[0] , argv[1]);

	
	max_size = strtoul(argv[2],NULL,0);
	
	if (max_size <= 0) 
	{
		printf("Error: size input\n");
		return EXIT_FAILURE;
	}

	max_size = 1 << (max_size -1);
	max_size = max_size * _2MB_TO_BYTE ;
	
			
	/* argv[1]: <name dev file> */
	memset(path_dev , '\0' , SIZE_PATH );
	sprintf(path_dev ,"%s%s" ,DIR_DEV, argv[1]);
	
	daemon_b = init_daemon_benchmark();

  
	memset(daemon_b->path_file_fl , '\0' , SIZE_PATH );
	sprintf(daemon_b->path_file_fl , "../risultati/%s_%lu.dat" , argv[1], strtoul(argv[2],NULL,0) );

	memset(daemon_b->path_file_synth , '\0' , SIZE_PATH );
	sprintf(daemon_b->path_file_synth , "../risultati/%s_%lu_synth.dat" , argv[1], strtoul(argv[2],NULL,0) );
	
	/* save the measures */
	if(record_measures(daemon_b , max_size , path_dev) < 0)
		goto err_rec;
		
	/* clean demon_benchmark */
  free(daemon_b);	
	
	printf("\nEnd %s for %s \n" , argv[0] , argv[1]);
	
	return EXIT_SUCCESS;

/* error mode */
err_rec:
  free(daemon_b);	
	return EXIT_FAILURE;
}
