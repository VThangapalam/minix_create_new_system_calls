#include <mqueuelib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, const char *argv[])
{
	if(argc < 2) {
		printf("need one argument,pid of receving process\n");
		printf("usage; test_file receiving_pid grp_id\n");
		exit(0);
	}

	
	mqd_t mqd = mq_open("queue1", O_WRONLY, 0, 5);
	printf("Queue ID: %d\n",mqd);

       
	message_t *mess = (message_t *)malloc(sizeof(message_t));
	mess->data = (char *)malloc(sizeof(char)*64);
	mess->receiver_pids = (int *)malloc(sizeof(int)*1);
int j=0;
int recvindex=0;
for(j=1;j<argc;j++)
{
const char *pid_rec = argv[j];
	int pid_int = atoi(pid_rec);
mess->receiver_pids[recvindex] = pid_int; 
recvindex++;
}

	
	mess->data = "hello 1";
	mess->sender_pid = getpid();
	mess->num_receivers = argc-1 ;

	int send_success = mq_send(mqd, mess, 64, 3);
	printf("Test send_success: %d\n", send_success);

}