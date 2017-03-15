#include <mqueuelib.h>
#include <unistd.h>
#include <signal.h>

void rec_exit(int sig) {
	int success;

         printf("Enter mqd to receive the message: ");
	mqd_t mqd;
	scanf("%d",&mqd);

	size_t buff_size = sizeof(char)*256;
	char *message_buff = (char *)malloc(buff_size);
	printf("Receiving from message queue %d\n", mqd);
	
       success = mq_receive(mqd, buff_size, message_buff, 3);

	printf("Success: %d\n", success);	
	if (success > 0) {
		printf("Here is the message: %s\n", message_buff);	

		//printf("Closing the queue: %d\n", mq_close(mqd));


}
	

}

int main(){

	

	pid_t my_pid = getpid();
	printf(" Application PID is %d\n", my_pid);


while(1)
{
signal(SIGUSR1,rec_exit);
mq_reqnotify(SIGUSR1);
printf("Waiting for signal SIGUSR1 (%d) for triggering receiving\n",SIGUSR1);
sleep(1000);
}
	

}