#include <mqueuelib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, const char *argv[])
{
	
	
	mqd_t mqd = mq_open("test5", O_WRONLY, 0, 5);
	printf("Test mqd: %d\n",mqd);

       //setting parameters int mq_setattr(mqd_t mqd, mq_attr_t *mq_attr)

        mq_attr_t *mq_attr=malloc(sizeof(mq_attr_t));
   char *name ;
name ="test5";
 mq_attr->name=name;
printf("name being sent is %s", mq_attr->name); 
        mq_attr->send_blocking=1;
mq_attr->receive_blocking=1;
mq_attr->max_messages=5;
mq_attr->max_message_size=10;

int res =mq_setattr(mqd,mq_attr);
printf("res of set %d\n",res);

//test get attribute
mq_attr_t *mq_attr_get=malloc(sizeof(mq_attr_t));
 mq_attr_get->name=name;
mqd_t mqd1 =-1;
int res1 =mq_getattr(mqd1,mq_attr_get);

printf("printing the parameters *****\n");
printf(" send block %d \n",mq_attr_get->send_blocking);
printf(" recv lock %d \n",mq_attr_get->receive_blocking);
printf(" max mess %d \n",mq_attr_get->max_messages);
printf(" max  mess size %d \n",mq_attr_get->max_message_size);

}