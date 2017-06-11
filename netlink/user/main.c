#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <stdlib.h>

#define NETLINK_TEST 25
#define MAX_PAYLOAD 1024

pthread_t pth_netlink;
pthread_mutex_t mutex;
int user_sockfd;

int netlink_receive_msg(char *data)
{
	return 0;
} 

void *pthread_netlink_receive(void *arg)
{
	struct nlmsghdr *nlh = NULL;
	struct sockaddr_nl dest_addr;
	struct iovec iov;
	struct msghdr msg;
	int ret;
	int sock_fd;

//	int (*func)(char *data) = arg;

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

	memset(&msg, 0, sizeof(msg));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD + 1);
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

//	sock_fd = (int)arg;

	while(1)
	{
		memset(nlh,0,NLMSG_SPACE(MAX_PAYLOAD));
		//ret = recvmsg(sock_fd,&msg,0);
		ret = recvmsg(user_sockfd,&msg,0);
		printf("receive kernel message:%s\n",NLMSG_DATA(nlh));	
#if 0
		if (ret > 0) {
			func(NLMSG_DATA(nlh));	
			break;
		}
#endif
	}

	free(nlh);
	pthread_exit("receive exit");
}

//int send_netlinkmsg(int sockfd, int flags, char *buf, int (*netlink_func)(char *data))
int send_netlinkmsg(int sockfd, int flags, char *buf)
{
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	int ret = -1;
//	pthread_t pth_netlink;

	//初始化目的地址信息
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK; 
	dest_addr.nl_pid = 0;   /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	//封装netlink消息头
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));	
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), buf);

	//将缓冲区向量iovec与消息进行绑定，指向消息头
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	//将要发送的消息与目的套接字绑定
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);  
	msg.msg_iov = &iov; 
	msg.msg_iovlen = 1;

	//发送信息
	sendmsg(sockfd,&msg,flags);

	//接收信息
//	recvmsg(sockfd,&msg,0);
//	printf("receive kernel message:%s\n",NLMSG_DATA(nlh));	
//	ret = pthread_create(&pth_netlink, NULL, pthread_netlink_receive, (void *)sockfd);
//	ret = pthread_create(&pth_netlink, NULL, pthread_netlink_receive, netlink_func);
	ret = pthread_create(&pth_netlink, NULL, pthread_netlink_receive, NULL);
	if (ret != 0) { 
		printf("pthread error!\n");
		return -1;
	}

	free(nlh);
	return 0;
}

int create_user_netlink_socket(long userpid)
{
	int sockfd;
	struct sockaddr_nl src_addr;
	
	sockfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);

	if(sockfd < 0) {
		return -1;
	}

	//初始化源地址信息
	memset(&src_addr, 0, sizeof(struct sockaddr_nl));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = userpid;
	src_addr.nl_groups = 0;

	//将本地套接字与源地址绑定
	if(bind(sockfd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
		close(sockfd);
		return -1;
	}
	
	return sockfd;
}

int main(int argc, char **argv)
{
//	int user_sockfd;
	int i;
	pid_t pid = getpid();

	user_sockfd = create_user_netlink_socket(pid);	
	if(user_sockfd < 0) {
		printf("user create socket error!\n");
		return -1;
	}

//	send_netlinkmsg(user_sockfd,0,"eeeee",netlink_receive_msg);
	send_netlinkmsg(user_sockfd,0,"eeeee");

	pthread_join(pth_netlink,NULL);	
	return 0;	
}
