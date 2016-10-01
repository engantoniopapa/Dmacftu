#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mibench.h"

#define UNLIMIT
#define MAXARRAY 60000 /* this number, if too large, will cause a seg. fault!! */

struct my3DVertexStruct {
  int x, y, z;
  double distance;
};

int compare_l(const void *elem1, const void *elem2)
{
  /* D = [(x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2]^(1/2) */
  /* sort based on distances from the origin... */

  double distance1, distance2;

  distance1 = (*((struct my3DVertexStruct *)elem1)).distance;
  distance2 = (*((struct my3DVertexStruct *)elem2)).distance;

  return (distance1 > distance2) ? 1 : ((distance1 == distance2) ? 0 : -1);
}


void start_qsort_large(int argc, char argv[N_ARG][SIZE_ARG] , struct mi_result *result)
{
  struct my3DVertexStruct array[MAXARRAY];
  struct perf_event_attr pe[2];
  int fd_pe[2];
  FILE *fp;
  int count=0;
  int x, y, z;
  
  if (argc<2) {
    fprintf(stderr,"Usage: qsort_large <file>\n");
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
    
    while((fscanf(fp, "%d", &x) == 1) && (fscanf(fp, "%d", &y) == 1) && (fscanf(fp, "%d", &z) == 1) &&  (count < MAXARRAY)) {
	 array[count].x = x;
	 array[count].y = y;
	 array[count].z = z;
	 array[count].distance = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	 count++;
    }
  }
  //printf("\nSorting %d vectors based on distance from the origin.\n\n",count);
  qsort(array,count,sizeof(struct my3DVertexStruct),compare_l);
  
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
