#include "daemon.h"
#include "core_daemon.h"

int main(int argc, char *argv[])
{
	unsigned long size; /* size of memory block */
	int type_operation;
	struct thread_benchmark th_b[N_CHAN];
	int ret;
	
	/* check on number of input parameter */
	if (argc != 3) 
	{
		printf("daemon <size memory buffer> <type_operation>\n");
		return EXIT_FAILURE;
	}
	
	printf("\nStart %s \n" , argv[0]);

	size = strtoul(argv[1],NULL,0);
	type_operation = atoi(argv[2]);

	if (size <= 0 || size > _MAX_BYTE) 
	{
		printf("Error: size input\n");
		return EXIT_FAILURE;
	}
	
	/* path dev files */
	int i;
	for( i = 0; i < N_CHAN ; ++i)
	{
		memset(th_b[i].path_dev , '\0' , SIZE_PATH );
		sprintf(th_b[i].path_dev ,"%s%d" ,DIR_DEV, i);
	}

	void *(*func_copy)(void *data);
	
	switch (type_operation) 
	{
		case TYPE_MEASURE_FROM:
		{
			func_copy = start_thread_from;
		}
		break;
		
		case TYPE_MEASURE_TO:
		{
			func_copy = start_thread_to;
		}
		break;
		
		case TYPE_MEASURE_FROM_TO:
		{
			func_copy = start_thread_from_to;
		}
		break;
		
		default:
		{
			printf("Error: type operation input\n");
			return EXIT_FAILURE;
		}
		break;
	}
	
	ret = start_micro_benchmark(th_b , size , func_copy);
	if(ret < 0)
		return EXIT_FAILURE;
	
	printf("\nEnd %s \n" , argv[0]);
	
	return EXIT_SUCCESS;
}
