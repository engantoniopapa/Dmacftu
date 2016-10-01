#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mibench.h"

#define UNLIMIT
#define MAXARRAY 60000 /* this number, if too large, will cause a seg. fault!! */

struct myStringStruct {
  char qstring[128];
};

int compare_s(const void *elem1, const void *elem2)
{
  int result;
  
  result = strcmp((*((struct myStringStruct *)elem1)).qstring, (*((struct myStringStruct *)elem2)).qstring);

  return (result < 0) ? 1 : ((result == 0) ? 0 : -1);
}


void start_qsort_small(int argc, char argv[N_ARG][SIZE_ARG] , struct mi_result *result) 
{
  struct myStringStruct array[MAXARRAY];
  struct perf_event_attr pe[2];
  int fd_pe[2];
  FILE *fp;
  int count=0;
  
  if (argc<2) {
    fprintf(stderr,"Usage: qsort_small <file>\n");
    exit(-1);
  }
  else {
		/* Performance count init */
		memset(&(pe[0]), 0, sizeof(struct perf_event_attr));
		memset(&(pe[1]), 0, sizeof(struct perf_event_attr));

		pe[0].type = PERF_TYPE_HARDWARE;
		pe[0].size = sizeof(struct perf_event_attr);
		pe[0].config = PERF_COUNT_HW_CACHE_REFERENCES;
		pe[0].disabled = 1;
		pe[0].exclude_kernel = 1;
		pe[0].exclude_hv = 1;
		
		
		pe[1].type = PERF_TYPE_HARDWARE;
		pe[1].size = sizeof(struct perf_event_attr);
		pe[1].config = PERF_COUNT_HW_CACHE_MISSES;
		pe[1].disabled = 1;
		pe[1].exclude_kernel = 1;
		pe[1].exclude_hv = 1;

		fd_pe[0] = perf_event_open(&(pe[0]), 0, -1, -1, 0);
		if (fd_pe[0] == -1) {
			 fprintf(stderr, "Error opening leader %llx\n", pe[0].config);
			 exit(EXIT_FAILURE);
		}
		
		fd_pe[1] = perf_event_open(&(pe[1]), 0, -1, -1, 0);
		if (fd_pe[1] == -1) {
			 fprintf(stderr, "Error opening leader %llx\n", pe[1].config);
			 close(fd_pe[0]);    	
			 exit(EXIT_FAILURE);
		}
		
		ioctl(fd_pe[0], PERF_EVENT_IOC_RESET, 0);
		ioctl(fd_pe[1], PERF_EVENT_IOC_RESET, 0);

		ioctl(fd_pe[0], PERF_EVENT_IOC_ENABLE, 0);
		ioctl(fd_pe[1], PERF_EVENT_IOC_ENABLE, 0);
	/*End Performance count init */
	
    fp = fopen(argv[1],"r");
    
    while((fscanf(fp, "%s", array[count].qstring) == 1) && (count < MAXARRAY)) {
	 count++;
    }
  }
  //printf("\nSorting %d elements.\n\n",count);
  qsort(array,count,sizeof(struct myStringStruct),compare_s);
  
  
  ioctl(fd_pe[0], PERF_EVENT_IOC_DISABLE, 0);
	ioctl(fd_pe[1], PERF_EVENT_IOC_DISABLE, 0);
	int ret;
	ret= read(fd_pe[0], &(result->cache_ref), sizeof(long long));
	if (ret == -1) {
		 fprintf(stderr, "Error read perf_event \n");
		 close(fd_pe[0]);    
		 close(fd_pe[1]); 
		 exit(EXIT_FAILURE);
	}
	
	ret = read(fd_pe[1], &(result->cache_miss), sizeof(long long));
	if (ret == -1) {
		 fprintf(stderr, "Error read perf_event \n");
		 close(fd_pe[0]);    
		 close(fd_pe[1]); 
		 exit(EXIT_FAILURE);
	}
  
  close(fd_pe[0]);    
	close(fd_pe[1]); 
}
