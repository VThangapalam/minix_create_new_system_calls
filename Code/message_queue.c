#include "mqueuelib.h"
#include "message_queue.h"

// tracking counters
static int curr_num_queues = 0;
static int curr_num_req = 0;

static int test_counter = 0;
static int group_id_counter = 1;

static mq_t queues[MAX_QUEUES];	/* list of open queues */
static int queues_mask[MAX_QUEUES] = {0};
static req_t req_receivers[MAX_REQ_NOT] = {0}; /* list of requests for notifications */


mqd_t do_mq_open()
{
printf("into open queue\n");
	size_t name_size = sizeof(char) * MAX_NAME_SIZE;
	char *name = (char *)malloc(name_size);
	sys_datacopy(who_e, (vir_bytes)m_in.m7_p1, SELF, (vir_bytes)name, name_size);
	int max_mess = m_in.m7_i3;
	uid_t my_uid = mproc[who_p].mp_realuid;
        int open_flag = m_in.m7_i1;

	 printf("do_mq_open: printing of variables\n");
	 printf("      name           %s\n", name);
    	 printf("      max_mess       %d\n", max_mess);

	if (curr_num_queues == MAX_QUEUES)
	{
		printf("Maximum number of queues have already been opened.\n");
		return FAIL;
	}

	pid_t calling_proc = mproc[who_p].mp_pid;

	if (calling_proc < 1)
	{
		printf("Unable to acquire the calling process's PID.\n");
		return FAIL;
	}

	if (open_flag != O_RDONLY && open_flag != O_WRONLY && open_flag != O_RDWR)
	{
		printf("The open flag must either be O_RDONLY, O_WRONLY, or O_RDWR.\n");
		return FAIL;
	}


	// printf("do_mq_open: name of queue: %s\n", name);
	int i;
	for (i = 0; i < curr_num_queues; i++)
	{
		if (strcmp(queues[i].attr->name, name) == 0)
		{
			if (open_flag == O_RDONLY)
			{
				int success = addproc(queues[i].receiver_pids, calling_proc);
				if (success == FAIL)
					return FAIL;
			}
			else if (open_flag == O_WRONLY)
			{
				int success = addproc(queues[i].sender_pids, calling_proc);
				if (success == FAIL)
					return FAIL;
			}
			else
			{
				int success = addproc(queues[i].sender_pids, calling_proc);
				if (success == FAIL)
					return FAIL;
				success = addproc(queues[i].receiver_pids, calling_proc);
				if (success == FAIL)
				{
					deleteproc(queues[i].sender_pids, calling_proc);
					return FAIL;
				}
			}
			return i;
		}
	}

	queues_mask[curr_num_queues++] = 1;

	 printf("do_mq_open: curr_num_queues %d\n", curr_num_req);

	mq_t *new_queue = malloc(sizeof(mq_t));
	new_queue->attr = malloc(sizeof(mq_attr_t));
	new_queue->attr->name = (char *)name;
	new_queue->attr->max_message_size = MAX_MESSAGE_SIZE;
	

	if (max_mess)
	{
		new_queue->attr->max_messages = max_mess;
		
	}
	else
	{
		new_queue->attr->max_messages = MAX_MESSAGES;
	}

	new_queue->curr_num_messages_total = 0;
	new_queue->sender_pids = malloc(sizeof(SIZE_INT * MAX_PROCESSES));
	new_queue->receiver_pids = malloc(sizeof(SIZE_INT * MAX_PROCESSES));
	new_queue->queue_high = malloc(sizeof(queue_t));
	new_queue->queue_norm = malloc(sizeof(queue_t));
	new_queue->queue_low = malloc(sizeof(queue_t));

	int num_messages = new_queue->attr->max_messages;
	int size_messages = new_queue->attr->max_message_size;

	// TODO: go back and fix this size of
	new_queue->queue_high->messages = malloc(sizeof(message_t) * num_messages * size_messages);
	new_queue->queue_norm->messages = malloc(sizeof(message_t) * num_messages * size_messages);
	new_queue->queue_low->messages = malloc(sizeof(message_t) * num_messages * size_messages);

	init_queue(new_queue->queue_high, num_messages);
	init_queue(new_queue->queue_norm, num_messages);
	init_queue(new_queue->queue_low, num_messages);

	queues[curr_num_queues - 1] = *new_queue;

	return curr_num_queues - 1;
	
}




int do_mq_send()
{
	mqd_t mqd = m_in.m7_i1;

	if (mqd >= curr_num_queues || mqd < 0) {
		printf("Wrong mq identifier (mqd)\n");
		return FAIL;
	} else if (queues_mask[mqd] == 0) {
		printf("Error: queue does not exist\n");
		return FAIL;
	}

	message_t *data = malloc(sizeof(message_t));
	size_t message_length = m_in.m7_i2;
	unsigned int priority = m_in.m7_i3;
unsigned int numberOfrecv = m_in.m7_i4;
        
	uid_t my_uid = mproc[who_p].mp_realuid;
	
	

	if (message_length > queues[mqd].attr->max_message_size)
	{
		printf("The length of the message is longer than we accept.\n");
		return FAIL;
	}

	char *pid_list = (char *)malloc(sizeof(char)*128);
	data->receiver_pids = malloc(sizeof(int)*m_in.m7_i4);
	data->data = malloc(sizeof(char)*message_length);
        data->num_receivers=malloc(sizeof(int));
        data->num_receivers = m_in.m7_i4;
printf("number of receivers to be sent to***** %d\n",data->num_receivers);


	sys_datacopy(who_e, (vir_bytes)m_in.m7_p1, SELF,(vir_bytes)pid_list, sizeof(char)*128);
	sys_datacopy(who_e, (vir_bytes)m_in.m7_p2, SELF,(vir_bytes)data->data, sizeof(char)*message_length);
//vt adding number of receivers per message 
sys_datacopy(who_e, (vir_bytes)m_in.m7_i4, SELF,(vir_bytes)data->num_receivers, sizeof(int));

printf("number of receivers to be sent to***** %d\n",data->num_receivers);


	data->sender_pid = atoi(strtok(pid_list,","));
	printf("do_mq_send: parsing sender_pid = %d\n", data->sender_pid);

	int i;
	for (i = 0; i < m_in.m7_i4; i++)
	{
		data->receiver_pids[i] = atoi(strtok(NULL,","));
		printf("do_mq_send: before enqueue, writing ********* data->receiver_pid[%d]=%d\n", i, data->receiver_pids[i]);
	}


	int success;
	switch (priority)
	{
		case 1:
			if (queues[mqd].queue_high->count == queues[mqd].queue_high->size)
			{
				printf("The queue is already full.\n");
				return FAIL;	// queue is full
			}
			success = enqueue(queues[mqd].queue_high, data);
			break;
		case 2:
			if (queues[mqd].queue_norm->count == queues[mqd].queue_norm->size)
			{
				printf("The queue is already full.\n");
				return FAIL;	// queue is full
			}
			success = enqueue(queues[mqd].queue_norm, data);
			break;
		case 3:
			if (queues[mqd].queue_low->count == queues[mqd].queue_low->size)
			{
				printf("The queue is already full.\n");
				return FAIL;	// queue is full
			}
			success = enqueue(queues[mqd].queue_low, data);
			break;
		default:
			return FAIL;
			break;
	}
	if (success == TRUE)
	{
printf("success!!! msg cnt ,%d\n" ,queues[mqd].queue_low->count);
printf("message data at 0 index in send %s \n",queues[mqd].queue_low->messages[0].data);
		queues[mqd].curr_num_messages_total++;
		//printf("do_mq_send: before notify_rec, data->receiver_pids[0]=%d\n", data->receiver_pids[]);
		notify_rec(data->receiver_pids);

	}

	printf("do_mq_send: success %d\n", success);
	return success;
	
}




void init_queue(queue_t *q, unsigned int num_messages)
{
	q->first = 0;
	if (num_messages == 0)
	{
		q->last = MAX_MESSAGES - 1;
		q->size = MAX_MESSAGES;
	}
	else
	{
		q->last = num_messages - 1;
		q->size = num_messages;
	}
	q->count = 0;
}

int enqueue(queue_t *q, message_t *msg)
{
	if (q->count >= q->size)
	{
		printf("Warning, queue overflow enqueue.\n");
		return FAIL;
	}
	else
	{
int lst =0;
//if(q->count!=0)
lst= q-> count;
q->messages[lst] = *msg;
q->count = q->count + 1;
  printf("number of messages in queue enqueue ! %d\n", q->count);
  printf("message enqueued ! %s\n", msg->data);
 printf("message enqueued from queue index ! %s\n", q->messages[lst].data);

		return TRUE;

 

	}
}


int do_mq_receive()
{
	printf("do_mq_receive: hello! before getting any variable...\n");
	mqd_t mqd = m_in.m1_i1;
	size_t buffer_length = (size_t)m_in.m1_i2;
	unsigned int priority = (unsigned int)m_in.m1_i3;

	if (mqd < 0 || queues_mask[mqd] == 0) {
		printf("Error: queue does not exist\n");
		return FAIL;
	}

	uid_t my_uid = mproc[who_p].mp_realuid;
printf("the receiver pid !!!!%d\n",my_uid);
	

	

	char *buffer_ptr = (char *)malloc(sizeof(char) * buffer_length);
	
	printf("do_mq_receive: after malloc before checking size...\n");

	if (buffer_length < queues[mqd].attr->max_message_size)
	{
		printf("The length of the buffer is not long enough for the message.\n");
		return FAIL;	// not a big enough buffer
	}
	
	printf("do_mq_receive: after buffer_length, before dequeueing...\n");

	int success;
	switch (priority)
	{
		case 1:
			if (queues[mqd].queue_high->count == 0)
			{
				printf("The queue is empty high*******.\n");
				return FALSE;	// no messages
			}
			success = dequeue(queues[mqd].queue_high, &buffer_ptr);
			break;
		case 2:
			if (queues[mqd].queue_norm->count == 0)
			{
				printf("The queue is empty norm.*****\n");
				return FALSE;	// no messages
			}
			success = dequeue(queues[mqd].queue_norm, &buffer_ptr);
			break;
		case 3:
			if (queues[mqd].queue_low->count == 0)
			{ 
				printf("The queue is empty low.*****\n");
				return FALSE;	// no messages
			}
			success = dequeue(queues[mqd].queue_low, &buffer_ptr);
			break;
		default:
			return FALSE;
			break;
	}
	if (success == TRUE)
	{
	 queues[mqd].curr_num_messages_total--;
         printf("current number of messages after receive@@@@@@ %d\n", queues[mqd].queue_low->count);
//added this        
 printf("do_mq_receive: dequeue of message: %s\n", buffer_ptr);

	sys_datacopy(SELF, (vir_bytes)buffer_ptr,
		who_e, (vir_bytes)m_in.m1_p1, queues[mqd].attr->max_message_size);
//copying message
	return TRUE;
 
	}
else
{
 printf("do_mq_receive: fail !!!!");
buffer_ptr="";
	sys_datacopy(SELF, "**no message to be received**",
		who_e, (vir_bytes)m_in.m1_p1, queues[mqd].attr->max_message_size);
return FALSE;
}

/*
	printf("do_mq_receive: dequeue of message: %s\n", buffer_ptr);

	sys_datacopy(SELF, (vir_bytes)buffer_ptr,
		who_e, (vir_bytes)m_in.m1_p1, queues[mqd].attr->max_message_size);

	return TRUE;
*/


}


int dequeue(queue_t *q, char **msg)
{
printf("into dequeue function\n");
//uid_t my_uid = mproc[who_p].mp_realuid;
pid_t my_uid=mproc[who_p].mp_pid;
printf("the receiver pid in dequeue !!!!%d\n",my_uid);
printf("1\n");
	if (q->count <= 0)
	{
		printf("Warning, empty queue dequeue.\n");
		return FAIL;
	}
	else
	{
printf("2\n");
              message_t *msg_t;
 for(int i=0;i<q->size;i++)
{
printf("3 queue size %d\n",q->size);
printf("value of i !!!!!! %d",i);
int queueShift=i;
     msg_t = &(q->messages[i]);
printf("4\n");
     for(int k=0;k<10; k++)
     {

//printf("5 k value !!!!!! is %d \n",k); 
printf("receiver pid %d in for loop is %d and the pid looking for is %d \n",k,msg_t->receiver_pids[k],my_uid);
          if( my_uid==msg_t->receiver_pids[k] && msg_t->receiver_pids[k]!=0)
          {
          printf("6\n");
             printf("retrieving msg %s\n", q->messages[i].data);
             	msg_t = &(q->messages[i]);
                *msg = msg_t->data;
             deleteproc(msg_t->receiver_pids, mproc[who_p].mp_pid);
printf("printing num of receivers before !!!%d \n",msg_t->num_receivers);
             msg_t->num_receivers=  msg_t->num_receivers-1;
printf("printing num of receivers after !!!*** %d \n",msg_t->num_receivers);

             if( msg_t->num_receivers==0 && i<q->size-1 && q->size!=1)
                 {
                 printf("7 shifting for!!!! i %d \n",i);
                 for( int j=queueShift;j<3;j++)
                    {
 printf("inside shifting for!!!! j %d \n",j);
                    message_t *msg_tsrc=&(q->messages[j+1]);
  
                    message_t *msg_tdest=&(q->messages[j]);
                     memcpy (msg_tdest,msg_tsrc, sizeof (message_t));
                  printf("8\n");
                  msg_tdest->data = msg_tsrc->data ;
                  
                  msg_tdest->sender_pid = msg_tsrc->sender_pid ;
                  msg_tdest->receiver_pids = msg_tsrc->receiver_pids;
                   msg_tdest->num_receivers = msg_tsrc->num_receivers;
                   printf("9\n");
                   q->count=q->count-1;
                   return TRUE;
                   
                   } //for for shifting messsges



                 }//if for checking if shift needed
else if (msg_t->num_receivers==0 && (i==q->size-1 || q->size==1 ))
{
printf("number of receivers 0 and dec ***q count \n"); 
q->count=q->count-1;
return TRUE;
} // no shifht required just dec count
             else
                {
printf("number of receivers more than one ! so message stil on queue not removed \n"); 
return TRUE;
                }//if there are more receivers
          }//if recev pid == mesg pid

      }// end of for loop for eseachinfg in meg for recv pid
               

   }
		


printf("no message found for the process !!\n");
return FALSE;	

}//else

	return  FALSE;
}
/* initprocs - Initialize the process list */
void initprocs(int *procs)
{
	int i;

	for (i = 0; i < MAX_PROCESSES; i++)
	{
		procs[i] = 0;
	}
}

/* addproc - Add a proc to the proc list */
int addproc(int *procs, pid_t pid)
{

printf("entered add proc \n");
	int i;

	if (pid < 1)
	{
		printf("Cannot add process of ID less than 1...\n");
		return FAIL;
	}

	for (i = 0; i < MAX_PROCESSES; i++) {
		if (procs[i] == 0)
		{
			procs[i] = pid;
			printf("Process registered ***** %d successfully...\n",procs[i]);
			return TRUE;
		}
	}
	printf("Tried to add too many processes...\n");
	return FAIL;
}



/* deleteproc - Delete a proc whose PID=pid from the proc list */
int deleteproc(int *procs, pid_t pid)
{
	int i;

	if (pid < 1)
	{
		printf("Cannot delete process of ID less than 1...\n");
		return FAIL;
	}

	for (i = 0; i < MAX_PROCESSES; i++)
	{
          //printf("proc in receive are %d\n",procs[i]);
		if (procs[i] == pid)
		{
			procs[i] = 0;
			printf("Process removed **** %d successfully...\n",pid);

			return TRUE;
		}
	}
	printf("Could not find the process %d you are trying to remove...\n",pid);
	return FAIL;
}


void notify_rec(int *receivers)
{
	
	int i, status;
	for (i = 0; i < MAX_REQ_NOT; i++)
	{

		if (req_receivers[i].pid > 0)
		{
                 printf("inside notify_rec..., argument req_receivers[%d]=%d\n",i,req_receivers[i]);
			printf("inside notify_rec... at least 1 pid (%d) requested notify\n", req_receivers[i].pid);

			status = array_search(receivers, MAX_PROCESSES, req_receivers[i].pid);
			if (status != FAIL)
			{
				printf("inside notify_rec... found receiver %d in array_search, executing kill\n",req_receivers[i].pid);

				int kill_status = sys_kill(req_receivers[i].proc_nr, req_receivers[i].signum);
				if ( kill_status < 0)
				{
					printf("Error %d: some proccess (%d) didn't receive the notification (%d)\n", kill_status, req_receivers[i].proc_nr, req_receivers[i].signum);
				}
else
{
printf("notify executed\n");
}
	

			}
		}
	}	
}

// returns the index of the match
int array_search(int *ary, int max, int element)
{
	int i = 0;

	while (i < max && ary[i] != element)
	{
		if (ary[i] > 0)
		{
			i++;
		}
		else
		{
			return FAIL;
		}
	}

	if (i < max)
	{
		return i;
	}
	else
	{
		return FAIL;
	}
}


int do_mq_reqnotify()
{
	req_t notification;
	notification.pid = mproc[who_p].mp_pid;
	notification.proc_nr = who_e;
	notification.signum = m_in.m1_i1;
	req_receivers[curr_num_req++] = notification;

	printf("proccess %d registered for signal %d\n", notification.pid, notification.signum);

	return 1;
}



int do_mq_close()
{
	mqd_t mqd = m_in.m1_i1;

	return(mq_close_helper(mqd));
}


int do_mq_setattr()
{
printf(" into set attribute\n");

size_t name_size = sizeof(char) * MAX_NAME_SIZE;
char *name = (char *)malloc(name_size);
sys_datacopy(who_e, (vir_bytes)m_in.m6_p1, SELF,(vir_bytes)name, name_size);
//name=m_in.m6_p1;
printf("the name received in set attribute is %s ",name);
long queue_id =-1;
int i=0;
for (i = 0; i < curr_num_queues; i++)
	{
		if (strcmp(queues[i].attr->name, name) == 0)
		{
                
                printf("the queue id for the queue %s is %d ths is system generate cannot set ",name,i);
          queue_id =i;
//check if current number of messages in queue is 0 only then attribute can be set to avoid loss of data
                printf("setting other parameters\n");
                printf("from user  send blocking %ld\n ",m_in.m6_l2);
               printf("from user  receive blocking %ld\n ",m_in.m6_l3);
printf("from user  max messages %d\n ",m_in.m6_s1);
printf("from user  message size %d\n",m_in.m6_s2);

// sys_datacopy(who_e, (int)(vir_bytes)m_in.m6_l2, SELF,(vir_bytes)queues[i].attr->send_blocking,sizeof(int));
// sys_datacopy(who_e, (int)(vir_bytes)m_in.m6_l3, SELF,(vir_bytes)queues[i].attr->receive_blocking,sizeof(int));
// sys_datacopy(who_e, (int)(vir_bytes)m_in.m6_s1, SELF,(vir_bytes)queues[i].attr->max_messages,sizeof(int));
 //sys_datacopy(who_e, (int)(vir_bytes)m_in.m6_s2, SELF,(vir_bytes)queues[i].attr->max_message_size,sizeof(int));
queues[i].attr->send_blocking=(int)m_in.m6_l2;
queues[i].attr->receive_blocking=(int)m_in.m6_l3;
queues[i].attr->max_messages=(int)m_in.m6_s1;
queues[i].attr->max_message_size=(int)m_in.m6_s2;
printf("after copy cheking send bloock %d\n",queues[i].attr->send_blocking);
printf("after copy cheking recv bloock %d\n",queues[i].attr->receive_blocking);

printf("after copy cheking max mess %d\n",queues[i].attr->max_messages);


printf("after copy cheking recv bloock %d\n",queues[i].attr->max_message_size);

         return 1;   

                }
          }

if(queue_id ==-1)
{
printf("the queue is not present cannot set attributes\n");
return -1;

}

	return 1;
}

int do_mq_getattr()
{
printf(" into get attribute\n");
size_t name_size = sizeof(char) * MAX_NAME_SIZE;
char *name = (char *)malloc(name_size);
sys_datacopy(who_e, (vir_bytes)m_in.m6_p1, SELF,(vir_bytes)name, name_size);
//name=m_in.m6_p1;
printf("the name received in get attribute is %s ",name);
long queue_id =-1;
int i=0;
for (i = 0; i < curr_num_queues; i++)
	{
		if (strcmp(queues[i].attr->name, name) == 0)
		{
                printf("the queue id for the queue %s is %d ",name,i);

printf("the send blocking parameter %d\n", queues[i].attr->send_blocking);
printf("the receive blocking parameter %d\n", queues[i].attr->receive_blocking);
printf("the max_messages parameter %d\n", queues[i].attr->max_messages);

printf("the max_message size parameter %d\n", queues[i].attr->max_message_size);
queue_id=i;
long *qid=&queue_id;
long *send =&(queues[i].attr->send_blocking);
long *recv =&(queues[i].attr->receive_blocking);
long *mess=&(queues[i].attr->max_messages);
long *messize =&(queues[i].attr->max_message_size);
//copying from pm to the user application
sys_datacopy(SELF, (vir_bytes)qid, who_e, (vir_bytes)m_in.m6_l1, sizeof(int));
sys_datacopy(SELF, (vir_bytes)send, who_e, (vir_bytes)m_in.m6_l2, sizeof(int));
sys_datacopy(SELF, (vir_bytes)recv, who_e, (vir_bytes)m_in.m6_l3, sizeof(int));
sys_datacopy(SELF, (vir_bytes)mess, who_e, (vir_bytes)m_in.m6_s1, sizeof(int));
sys_datacopy(SELF, (vir_bytes)messize, who_e, (vir_bytes)m_in.m6_s2, sizeof(int));
printf("  send block after sys copy m_in.m6_l2 %ld ",m_in.m6_l2);
return 1;

                }
        }

if(queue_id ==-1)
{
printf("no such queue found\n");
return -2;
}		


	return 1;
}


int mq_close_helper(mqd_t mqd) {

	if ( mqd < 0 || queues_mask[mqd] == 0) {
		printf("Error: queue does not exist\n");
		return FALSE;
	}
	uid_t my_uid = mproc[who_p].mp_realuid;
	

	

	int success = deleteproc(queues[mqd].sender_pids, mproc[who_p].mp_pid);
	int success_sender = emptyprocs(queues[mqd].sender_pids);
	success = deleteproc(queues[mqd].receiver_pids, mproc[who_p].mp_pid);
	int success_receiver = emptyprocs(queues[mqd].receiver_pids);

	if (success_sender == TRUE && success_receiver == TRUE)
	{
		free(queues[mqd].attr);
		free(queues[mqd].sender_pids);
		free(queues[mqd].receiver_pids);
		free(queues[mqd].queue_high->messages);
		free(queues[mqd].queue_norm->messages);
		free(queues[mqd].queue_low->messages);
		free(queues[mqd].queue_high);
		free(queues[mqd].queue_norm);
		free(queues[mqd].queue_low);
		free(&queues[mqd]);
	}

	queues_mask[mqd] = 0;

	printf("Queue [%d] closed succesfully\n", mqd);

	return TRUE;
}


int emptyprocs(int *procs)
{
	int i;
       int  isempty = 0;
	for (i = 0; i < MAX_PROCESSES; i++)
	{
		/*if (procs[i] != 0)
		{
			return FALSE;
		}
*/

              if(procs[i]==0)
              isempty=1;
              else 
              isempty=0;


	}
printf("the queue is now empty@@@@@@@\n");
  	//return TRUE;
return isempty;
}
