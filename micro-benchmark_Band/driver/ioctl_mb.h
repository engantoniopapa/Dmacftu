#ifndef _IOCTL_MB_H_
#define _IOCTL_MB_H_

#include <linux/ioctl.h>

/** for pin/unpin user pages -
 * write: whether pages will be written to by the caller 
 * buf_user: starting user address
 * len: size of user buffer
*/
typedef struct
{
	unsigned short int write;
  char  *buf_user;  
  size_t len;
} query_arg_t;

#define MB_IOC_MAGIC 'q'
#define MB_LOAD _IOW(MB_IOC_MAGIC , 1, query_arg_t *) 
#define MB_UNLOAD _IOW(MB_IOC_MAGIC , 2,  query_arg_t *)

#endif
