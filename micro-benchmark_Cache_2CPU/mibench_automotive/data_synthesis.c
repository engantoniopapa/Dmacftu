#include "mibench.h"
#include <math.h>

/* compare function for qsort() for unsigned long long*/
static int comp_ull(const void * a, const void * b)
{
  if (*(unsigned long long*)a == *(unsigned long long*)b)
    return 0;

  if (*(unsigned long long*)a < *(unsigned long long*)b)
      return -1;
    
  return 1;
}

/* compare function for qsort() for double*/
/*static int comp_d(const void * a, const void * b)
{
  if (*(double*)a == *(double*)b)
    return 0;

  if (*(double*)a < *(double*)b)
      return -1;
    
  return 1;
}*/


/** get the average for unsigned long long
 * @arr: list element
 * @n: size of @arr
 * return the average.
*/
static double getAvg_ull(unsigned long long arr[], unsigned int n)
{
	double avg;
	unsigned long long sum = 0;
	int i;
	
	for(i = 0; i < n; ++i)
		sum += arr[i];
	
	avg = ((double) sum ) / n;
	
	return avg; 
}


/** get the average for double
 * @arr: list element
 * @n: size of @arr
 * return the average.
*/
static double getAvg_d(double arr[], unsigned int n)
{
	double avg;
	double sum = 0;
	int i;
	
	for(i = 0; i < n; ++i)
		sum += arr[i];
	
	avg =  sum / n;
	
	return avg; 
}

/** get Variance
 * @arr: list element
 * @n: size of @arr
 * @avg: the average of @arr
 * return the variance.
*/
/*static double getVar(unsigned long long arr[], unsigned int n , double avg)
{
	double var = 0;
	double tmp_diff;
	int i ;
	
	for(i = 0 ; i < n ; ++i)
	{
		tmp_diff = ((double) arr[i]) - avg ;
		var = var + (pow(tmp_diff , 2) / (n-1) );
	}
	return var;
}*/

/** synthesis of measures
 * @mi_state: main descriptor for test
 * @time_op: list of element to summarize 
 * @per_cache_miss: list of element to summarize 
 * @name_test: type test to summarize
 * return: zero on success or negative value for error
*/  
int data_synthesis( struct mibench_state* mi_state, unsigned long long *time_diff , double *per_cache_miss,  char *name_test)
{
	double avg_diff;
	double avg_miss;
	//double var; 
	//double median;
	//unsigned long long min;
	unsigned long long max_diff;
	double avg_cache_ref;
	char line_buf[N_CHAR_LINE];
	int ret;
	int len;

	qsort(time_diff, mi_state->size , sizeof(unsigned long long), comp_ull);
	//qsort(per_cache_miss, count , sizeof(unsigned long long), comp_d);
	
	//min = time_diff[0];
	max_diff = time_diff[mi_state->size -1];
	//var = getVar(time_diff, mi_state->size  , avg);

	avg_diff = getAvg_ull(time_diff, mi_state->size );
	avg_miss = getAvg_d(per_cache_miss, mi_state->size );

	int i ;
	avg_cache_ref = 0;
	for(i = 0 ; i < mi_state->size; ++i)
	{
		avg_cache_ref = avg_cache_ref + mi_state->results[i].cache_ref;
	}
	avg_cache_ref = avg_cache_ref / mi_state->size;

	// one element for median
	/*if( mi_state->size  %2)
	{
		median = time_diff[mi_state->size /2];
	}	
	else // two elements for median
	{
		median = (double) (time_diff[(mi_state->size /2) -1] + time_diff[(mi_state->size /2)]);
		median = median /2;
	}*/
	
	memset(line_buf, '\0' , N_CHAR_LINE);	
		
	sprintf(line_buf," %23s%8u%25.4f%25llu%20.4f%16.6f \n", name_test, mi_state->size , 
	avg_diff/MICROSEC_TO_NS , max_diff/MICROSEC_TO_NS , avg_cache_ref ,avg_miss);	
	len = strlen(line_buf);
	
	while( len != 0)
	{
		ret = write(mi_state->fd_synthesis, line_buf , len);
		if(ret < 0)
		{
			perror("error write file"); 
			return -EFAULT ;
		}
		
		len = len - ret;
	}
	return 0;
}
