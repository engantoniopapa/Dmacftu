#ifndef _DAEMON_OPERATION_H_
#define _DAEMON_OPERATION_H_

int daemon_create_sparse_file(struct daemon_trace *daemon_ptr , int flags , mode_t mode);

int sparse_first_row(struct daemon_trace *daemon_ptr);

int copy_normal_file(struct daemon_trace *daemon_ptr);

#endif
