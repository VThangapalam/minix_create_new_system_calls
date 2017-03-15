#include "pm.h"
#include "mproc.h"
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define MAX_QUEUES		16	/* maximum number of allowed open queues */
#define MAX_MESSAGES		16	/* maximum number of messages in the queue */
#define MAX_MESSAGE_SIZE	256	/* maximum size of a message stored in the queue */
#define MAX_PROCESSES		16	/* maximum number of processes allowed to read/write to a queue */
#define MAX_REQ_NOT		MAX_PROCESSES*MAX_QUEUES /* maximum amount of notify requests */
#define SIZE_CHAR		8	/* default size of a char value */
#define SIZE_INT		4	/* default size of an int value */
#define MAX_NAME_SIZE		64	/* maximum number of chars of queue name */
#define MAX_ADMINS			32

#define FAIL			-1
#define FALSE			0
#define TRUE			1

void init_queue(queue_t *q, unsigned int max_messages);

int enqueue(queue_t *q, message_t *msg);

int dequeue(queue_t *q, char **msg);

void initprocs(int *procs);

int addproc(int *procs, pid_t pid);

int deleteproc(int *procs, pid_t pid);

int emptyprocs(int *procs);

int addprocInQueue(int *procs, pid_t pid,queue_t *q);
int deleteprocInQueue(int *procs, pid_t pid,int num_recv);
int emptyprocsInQueue(int *procs,queue_t *q);

/* send signal to receivers that requested to be notified */
void notify_rec(int *receivers); 

/* search int element in the specified int array */
int array_search(int *ary, int max, int element);


int mq_close_helper(mqd_t mqd);