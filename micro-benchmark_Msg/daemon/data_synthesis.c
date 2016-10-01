#include "daemon.h"

/* compare function for qsort() */
static int comp(const void * a, const void * b)
{
  if (*(unsigned long long*)a == *(unsigned long long*)b)
    return 0;

  if (*(unsigned long long*)a < *(unsigned long long*)b)
      return -1;
    
  return 1;
}

/** get the average
 * @arr: list element
 * @n: size of @arr
 * return the average.
*/
static double getAvg(unsigned long long arr[], unsigned int n)
{
	double avg;
	unsigned long long sum = 0;
	int i;
	
	for(i = 0; i < n; ++i)
		sum += arr[i];
	
	avg = ((double) sum ) / n;
	
	return avg; 
}

/** get Variance
 * @arr: list element
 * @n: size of @arr
 * @avg: the average of @arr
 * return the variance.
*/
static double getVar(unsigned long long arr[], unsigned int n , double avg)
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
}

/** synthesis of measures
 * @fd: where write synthesis
 * @count: element size of @time_op
 * @time_op: list of element to summarize 
 * @size: size of memory copy to summarize
 * return: zero on success or negative value for error
*/  
int data_synthesis(int fd , unsigned int count , unsigned long long *time_op , unsigned long size)
{
	double avg;
	double var; 
	double median;
	unsigned long long min;
	unsigned long long max;
	char line_buf[N_CHAR_LINE];
	int ret;
	int len;
	
	qsort(time_op, count , sizeof(unsigned long long), comp);
	
	min = time_op[0];
	max = time_op[count -1];
	avg = getAvg(time_op, count);
	var = getVar(time_op, count , avg);

	// one element for median
	if( count %2)
	{
		median = time_op[count/2];
	}	
	else // two elements for median
	{
		median = (double) (time_op[(count/2) -1] + time_op[(count/2)]);
		median = median /2;
	}
	
	memset(line_buf, '\0' , N_CHAR_LINE);	
	sprintf(line_buf," %16lu%14llu%14llu%20.4f%22.4f%20.4f \n", size,min , max , avg , var , median);
	len = strlen(line_buf);
	
	while( len != 0)
	{
		ret = write(fd, line_buf , len);
		if(ret < 0)
		{
			perror("error write file"); 
			return -EFAULT ;
		}
		
		len = len - ret;
	}
	return 0;
}
