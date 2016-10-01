#include "daemon.h"
#include "core_daemon.h"

int main(int argc, char *argv[])
{
	unsigned long max_size; /* max size byte */
	struct thread_benchmark th_b[N_CHAN];
	int ret;
	
	/* check on number of input parameter */
	if (argc != 3) 
	{
		printf("daemon <max byte message> <n_op>\n");
		return EXIT_FAILURE;
	}
	
	printf("\nStart %s \n" , argv[0]);

	max_size = strtoul(argv[1],NULL,0);

	if (max_size <= 0 || max_size > 8) 
	{
		printf("Error: size input\n");
		return EXIT_FAILURE;
	}

	max_size = 1 << (max_size -1);
	max_size = max_size * _2MB_TO_BYTE ;
	
	/* path dev files */
	int i;
	for( i = 0; i < N_CHAN ; ++i)
	{
		th_b[i].n_op = strtoul(argv[2],NULL,0);

		memset(th_b[i].path_dev , '\0' , SIZE_PATH );
		sprintf(th_b[i].path_dev ,"%s%d" ,DIR_DEV, i);
	}

	void *(*func_copy)(void *data);

	func_copy = start_thread_from;
	ret = start_micro_benchmark(th_b , max_size , func_copy ,DIR_RESULT_FROM , TYPE_MEASURE_FROM);	
	if(ret < 0)
		return EXIT_FAILURE;

	func_copy = start_thread_to;
	ret = start_micro_benchmark(th_b , max_size , func_copy ,DIR_RESULT_TO , TYPE_MEASURE_TO);
	if(ret < 0)
		return EXIT_FAILURE;

	func_copy = start_thread_from_to;
	ret = start_micro_benchmark(th_b , max_size , func_copy ,DIR_RESULT_FROM_TO , TYPE_MEASURE_FROM_TO);
	if(ret < 0)
		return EXIT_FAILURE;
	
	printf("\nEnd %s \n" , argv[0]);
	
	return EXIT_SUCCESS;
}
