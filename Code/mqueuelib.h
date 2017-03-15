#ifndef _MQUEUELIB_H_
#define _MQUEUELIB_H_

#include <lib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int mqd_t;

/* the message structure for sending and receiving messages */
typedef struct {
	char *data;			/* data of the message */
	int sender_pid;			/* process sending the message */
	int *receiver_pids;		/* process(es) receiving the message */
	int num_receivers;
} message_t;

/* the message queue structure */
typedef struct {
	message_t *messages;		/* messages of queue */
	int first;                      /* position of first element */
	int last;                       /* position of last element */
	int count;                      /* number of queue elements */
	int size;			/* maximum size of the queue */
} queue_t;

/* the attribute structure for the message queue attributes */
typedef struct {
	char* name;			/* name of the message queue */
	int send_blocking;		/* is send blocking? */
	int receive_blocking;		/* is receiving blocking? */
	int max_messages;		/* number of messages that can be stored in each sub-queue */
	int max_message_size;		/* maximum size of the message */
} mq_attr_t;

/* the message queue structure */
typedef struct {
	mq_attr_t *attr;		/* the message queue attributes */
	int *sender_pids;		/* the processes sending messages to this queue */
	int *receiver_pids;		/* the processes receiving messages from this queue */
	int curr_num_messages_total;	/* the total number of messages in this queue */
	queue_t* queue_high;		/* the high priority queue */
	queue_t* queue_norm;		/* the regular priority queue */
	queue_t* queue_low;		/* the low priority queue*/
} mq_t;

typedef struct {
	endpoint_t proc_nr;
	int pid;			/* PID of the requester */
	int signum; 			/* the signal that should be sended to the process that request notification*/
} req_t;



/* Open will take attributes that:
	name = describe the name of the queue
	open_flag = will the process read, write, or do both transactions to the queue:
		O_RDONLY | O_WRONLY | O_RDWR
	blocking_flag = will the queue be blocking or be asynchronous
	mq_attr = will the user define specific attributes for this queue */
/* Returns the queue number for the process to reference */
mqd_t mq_open(const char *name, int open_flag, int blocking_flag, int max_mess)
{
	message m;
	m.m7_i1 = open_flag;
	m.m7_i2 = blocking_flag;
	m.m7_i3 = max_mess;
	m.m7_p1 = name;
	return(_syscall(PM_PROC_NR, 35, &m));
}

/* Process wants to close its connection to the queue */
/* Returns TRUE if successful, FAIL otherwise */
int mq_close(mqd_t mqd)
{
	message m;
	m.m1_i1 = (int)mqd;
	return(_syscall(PM_PROC_NR, 44, &m));
}

/* Process wants to change the attributes of the queue */
/* Returns TRUE if successful, FAIL otherwise */
int mq_setattr(mqd_t mqd, mq_attr_t *mq_attr)
{

	message m;
	m.m6_l1 = (long)mqd;
	m.m6_l2 = (long)mq_attr->send_blocking;
	m.m6_l3 = (long)mq_attr->receive_blocking;
	m.m6_s1 = (short)mq_attr->max_messages;
	m.m6_s2 = (short)mq_attr->max_message_size;
	m.m6_p1 = mq_attr->name;

	return(_syscall(PM_PROC_NR, 56, &m));
}

/* Process wants to see the attributes of the queue */
/* Returns the attribute structure mq_attr_t */
int mq_getattr(mqd_t mqd, mq_attr_t *mq_attr)
{
/*
	message m;
	
	m.m1_i1 = mqd;
	char *str_ptr = (char *)malloc(sizeof(char) * 512);
	m.m1_p1 = str_ptr;

	int status = _syscall(PM_PROC_NR, 58, &m);

	mq_attr->name = strtok(str_ptr,",");
	mq_attr->send_blocking = atoi(strtok(NULL,","));
	mq_attr->receive_blocking = atoi(strtok(NULL,","));
	mq_attr->max_messages = atoi(strtok(NULL,","));
	mq_attr->max_message_size = atoi(strtok(NULL,","));
	

	return(status);
*/

message m;
	m.m6_l1 = mqd;
	m.m6_l2 = mq_attr->send_blocking;
	m.m6_l3 = mq_attr->receive_blocking;
	m.m6_s1 = mq_attr->max_messages;
	m.m6_s2 = mq_attr->max_message_size;
	m.m6_p1 = mq_attr->name;

	return(_syscall(PM_PROC_NR, 58, &m));

}

/* Process wants to send a message:
	mqd = queue number
	data = message structure to store in the queue
	message_length = length of the message to be sent
	priority = the priority of the message
*/
/* Returns TRUE if successful, FAIL otherwise */
int mq_send(mqd_t mqd, message_t *data, size_t message_length, unsigned int priority)
{
	message m;
	m.m7_i1 = (int)mqd;
	m.m7_i2 = (int)message_length;
	m.m7_i3 = (int)priority;
	m.m7_i4 = data->num_receivers;
	int mq_for_loop_counter;
	char pid_list[128]; // 5(pid length) * 16(MAX_PROCCESS) + 15(comma separated) < 128
	snprintf(pid_list, 128, "%d", data->sender_pid);

	for (mq_for_loop_counter = 0; mq_for_loop_counter < m.m7_i4; mq_for_loop_counter++)
	{
		snprintf(pid_list, 128, "%s,%d", pid_list, data->receiver_pids[mq_for_loop_counter]);
	}
	m.m7_p1 = pid_list;
	m.m7_p2 = data->data;
	return(_syscall(PM_PROC_NR, 69, &m));
}


/* Process wants to receive a message:
	mqd = queue number
	buffer_ptr = pointer to the buffer where the message should be saved
	buffer_length = length of available space in the buffer
	priority = the priority of the message which is being received
*/
/* Returns the message if successfull, if not returns empty string */
int mq_receive(mqd_t mqd, size_t buffer_length, char *buffer, unsigned int priority)
{
	message m;
	m.m1_i1 = (int)mqd;
	m.m1_i2 = (int)buffer_length;
	m.m1_i3 = (int)priority;
	m.m1_p1 = buffer;
	return(_syscall(PM_PROC_NR, 70, &m));
}

/* Process may specificy how it would like to be notified about new messages in the queue */
int mq_reqnotify(int signum)
{
	message m;
	m.m1_i1 = signum;
	return(_syscall(PM_PROC_NR, 79, &m));
}

#endif /* _MQUEUELIB_H_ */