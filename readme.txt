Step to include the new system calls

1. Go to /usr/src/servers/pm. In table.c file add the following system calls
do_mq_open //35
do_mq_close //44
do_mq_setattr//56
do_mq_getattr//58
do_mq_send//69
do_mq_receive//70
do_mq_reqnotify//79

2. Go to /usr/src/servers/pm. In the proto.h file add the following system call prototypes
mqd_t do_mq_open(void);
int do_mq_close(void);
int do_mq_setattr(void);
int do_mq_getattr(void);
int do_mq_send(void);
int do_mq_receive(void);
int do_mq_reqnotify(void);

3. Implementations of the system call can be found under message_queue.c file which is also under 
/usr/src/servers/pm.

4. To define global variables and function prototypes the header file message_queue.h file is used. 
The location of this file is /usr/src/servers/pm.

5. The different data structures used for the implementations are defined in filq mqueuelib.h
   under /usr/src/include add the file in the Make file in the location

5. Add the message_queue.c file to the /usr/src/servers/pm/Makefile  SRCS  so that the new file is compiled

6. To rebuild the minix with the new system calls go to
   /usr/src/releasetools and type 
   -> make services
   -> make install 
   -> reboot

7. Now you can test the new system calls with send.c and recv.c 
  Steps to run the sender 
Compile program 
//receiver program
cc recv.c
./a.out 

//sender program can change msg to be sent andits priority also message can be sent to multiple receivers
cc send.c
./a.out <receiver pid>


  

    
 


