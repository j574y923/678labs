#ifndef JOB_STRUCT
#define JOB_STRUCT
#include "pid_queue.h"

typedef struct{
	int job_id;
	int pid;
	char *cmd;
	pid_queue* process_queue;
}job_struct;

#endif

