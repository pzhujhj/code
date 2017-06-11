#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/fs.h>

/*1. 包含的头文件：linux/timer.h

2. 数据类型：struct timer_list;

包含的主要成员：
a. data:传递到超时处理函数的参数，主要在多个定时器同时使用时，区别是哪个timer超时。
b. expires:定时器超时的时间，以linux的jiffies来衡量。
c. void (*function)(unsigned long):定时器超时处理函数。
3. 主要相关的API函数：

a. init_timer(struct timer_list*):定时器初始化函数；
b. add_timer(struct timer_list*):往系统添加定时器；
c. mod_timer(struct timer_list *, unsigned long jiffier_timerout):修改定时器的超时时间为jiffies_timerout;
d. timer_pending(struct timer_list *):定时器状态查询，如果在系统的定时器列表中则返回1，否则返回0；
e. del_timer(struct timer_list*):删除定时器。
4. 时间与jiffies的转换函数：

Linux系统中的jiffies类似于Windows里面的TickCount,它是定义在内核里面的一个全局变量，只是它的单位并不是秒或是毫秒。通常是250个jiffies为一秒，在内核里面可以直接使用宏定义：HZ。这里有几个时间和jiffies的相互转换函数：
unsigned int jiffies_to_msecs(unsigned long);
unsigned int jiffies_to_usecs(unsigned long);
unsigned long msecs_to_jiffies(unsigned int);
unsigned long usecs_to_jiffies(unsigned int);*/

struct timer_list timer;

static void timer_handler(unsigned long data)
{
	char *buf = NULL;
	buf = (char *)data;//如果是传的字符，则需要强制转换
	
	/*修改定时器的超时参数并重启*/
	mod_timer(&timer, jiffies + 6*HZ);
}

static int __init timer_init(void)
{
	/*初始化定时器*/
	init_timer(&timer);
	//下面三个参数需要我们自己填充
	timer.expires = jiffies + HZ;
	timer.data = "tttt";
	//timer.data = 0;
	timer.function = timer_handler;
	
	/*添加激活计时器*/
	add_timer(&timer);
	return 0;
}

static void __exit timer_exit(void)
{
	del_timer(&timer);
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
