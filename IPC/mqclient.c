#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define MSG_PATH "/home/jihj/Mystudy/test.que"
#define MAX_MSG 1024

struct msgbuf {
	long mtype;
	char mtext[MAX_MSG];
};


static void send_msg(int qid, int msgtype)
{
	struct msgbuf msg;
	time_t t;

	msg.mtype = msgtype;

	time(&t);
	snprintf(msg.mtext, sizeof(msg.mtext), "xxxxxa message at %s",
			ctime(&t));

	if (msgsnd(qid, (void *) &msg, MAX_MSG,
				IPC_NOWAIT) == -1) {
		perror("msgsnd error");
		exit(EXIT_FAILURE);
	}
	printf("sent: %s", msg.mtext);
}


int main()
{
	int msgid = -1;
	key_t key;

	/*系统建立IPC通讯 （消息队列、信号量和共享内存） 时必须指定一个ID值。通常情况下，该id值通过ftok函数得到*/
	key = ftok(MSG_PATH, 'T');
	if (key < 0) {
		if (errno != ENOENT) {
			printf("Failed to ftok: %s\n", strerror(errno));
		}

		int fd;
		fd = open(MSG_PATH, O_CREAT, 0644);
		if (fd < 0) {
			printf("Failed to create %s: %s\n", MSG_PATH, strerror(errno));
		}
		
		close(fd);
	}

	msgid=msgget(key, IPC_EXCL);  /*检查消息队列是否存在*/  
	if(msgid < 0) {  
	    msgid = msgget(key, IPC_CREAT|0666);/*创建消息队列*/  
	    if(msgid <0) {  
		    printf("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));  
		    exit(-1);
	    }
    }

	/*发送消息队列的信息*/
	send_msg(msgid, 1);//注意这里第二个参数需要传一个大于0的数

	/*清除掉消息队列*/
	if (msgid > 0) {
		msgctl(msgid, IPC_RMID, NULL);
		msgid = -1;
	}

	return 0;
}


