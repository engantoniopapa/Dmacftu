#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mibench.h"

int main(int argc, char *argv[])
{
	char path_measure[SIZE_PATH]; /* path measures file */ 
	char path_synthesis[SIZE_PATH]; /* path synthesis file */ 
	struct mibench_state mi_state; /* main descriptor */
	int ret;
	int flag = O_RDWR | O_CREAT | O_TRUNC ;


	/* check on number of input parameter */
	if (argc != 3) 
	{
		printf("mibench <n_test> <name file>\n");
		return EXIT_FAILURE;
	}
	
	mi_state.size = atoi(argv[1]);
	
	memset(path_measure , '\0' , SIZE_PATH );
	memset(path_synthesis , '\0' , SIZE_PATH );

	
	sprintf(path_measure,"../risultati/measures_%s.dat" , argv[2]);
	sprintf(path_synthesis,"../risultati/synthesis_%s.dat" , argv[2]);

	mi_state.fd_measure = open(path_measure,flag, (mode_t)0666);
	if(mi_state.fd_measure < 0)
	{
		perror("error open measure file");
		return EXIT_FAILURE;
	}
	
	mi_state.fd_synthesis = open(path_synthesis,flag, (mode_t)0666);
	if(mi_state.fd_synthesis < 0)
	{
		perror("error open synthesis file");
		goto err_main_1;
	}

	mi_state.results = (struct mi_result *) malloc ( sizeof(struct mi_result) * mi_state.size);
	if(!mi_state.results)
	{
		perror("error init memory results");
		goto err_main_2;
	}

	ret = init_test(&mi_state);
	if(ret !=  0)
	{
		printf("error start_measure() \n");
		goto err_main_3;
	}

	free(mi_state.results);
	close(mi_state.fd_synthesis);
	close(mi_state.fd_measure);
	return EXIT_SUCCESS;

err_main_3:
	free(mi_state.results);
	
err_main_2:
	close(mi_state.fd_synthesis);
	
err_main_1:
	close(mi_state.fd_measure);
	return EXIT_FAILURE;

} 
