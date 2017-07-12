/*************************************************************      
    FileName : mysigaction.c  
    FileFunc : 1秒产生一次信号处理     
    Version  : V0.1      
    Author   :      
    Date     : 
    Descp    : Linux下信号处理

相关函数：

struct itimerval {  
    struct timeval it_interval; //计时器重启动的间歇值
    struct timeval it_value;    //计时器安装后首先启动的初始值
};  
  
struct timeval {  
    long tv_sec;                // 秒 
    long tv_usec;               // 微妙(1/1000000)
};

int setitimer(int which, const struct itimerval *value,struct itimerval *ovalue);
setitimer()将value指向的结构体设为计时器的当前值，如果ovalue不是NULL，将返回计时器原有值

which：间歇计时器类型，有三种选择
ITIMER_REAL      //数值为0，计时器的值实时递减，发送的信号是SIGALRM。
ITIMER_VIRTUAL //数值为1，进程执行时递减计时器的值，发送的信号是SIGVTALRM。
ITIMER_PROF     //数值为2，进程和系统执行时都递减计时器的值，发送的信号是SIGPROF

返回说明： 
成功执行时，返回0。失败返回-1，errno被设为以下的某个值 
EFAULT：value或ovalue是不有效的指针
EINVAL：其值不是ITIMER_REAL，ITIMER_VIRTUAL 或 ITIMER_PROF之一

取消定时器
setitimer设置it_interval和it_value为零。
void uninit_time() 
{ 
    struct itimerval value; 
    value.it_value.tv_sec = 0; 
    value.it_value.tv_usec = 0; 
    value.it_interval = value.it_value; 
    setitimer(ITIMER_REAL, &value, NULL); 
} 
	
*************************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>

void sig_prof(int signo)
{
	printf("Signal number is %d !\n",signo);
}

void init_sigaction(void)
{
	struct sigaction act;
	act.sa_handler=sig_prof;
	act.sa_flags=0;
	sigemptyset(&act.sa_mask);//初始化信号集
	//sigaction(SIGPROF,&act,NULL);//SIGPROF表示使用setitimer指定的Profiling Interval Timer所产生 
	sigaction(SIGALRM,&act,NULL);
}

void set_timer(void)
{
	struct itimerval itv;
	itv.it_value.tv_sec=1;//秒  1s后第一次启动定时器
	itv.it_value.tv_usec=0;//微秒
	//value.it_interval=value.it_value;//间隔时间,如果不指定就只实现一次定时,如果it_value和it_interval两者都清零,则会清除定时器
	itv.it_interval.tv_sec = 1; //启动后每隔一秒重新执行
    itv.it_interval.tv_usec = 0;
	
	//setitimer(ITIMER_PROF,&itv,NULL);//ITIMER_PROF送出SIGPROF信号
	setitimer(ITIMER_REAL,&itv,NULL);
}

int main(int argc,char *argv[])
{
	init_sigaction();//初始化设置信号处理
	set_timer();//精确定时
	while(1);
	return 0;
	//exit(0);
}
