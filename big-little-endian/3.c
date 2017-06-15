#include <stdio.h>

/*联合体union存放顺序是所有成员都从低地址存放*/

int main()
{
#if 0
	union {
		short int a;
		char b;
		short int c;
	}temp;

	temp.a = 0x1234;

	printf("%x\n", temp.a);
	printf("%x\n", temp.b);
	printf("%x\n", temp.c);
	if(temp.b == 0x12)
		printf("big\n");
	else 
		printf("litlle\n");
#else
	union w {
		int a;
		char b;
	}c;

	//C语言为共用体c分配了4个字节的内存空间，可以想象成4个格子，
	//变量a占4个，变量b只占低地址的一个,因为变量都是从低地址开始存储的
	c.a = 1;
	
	printf("%d\n", c.b);
	if (c.b == 1)
		printf("小端模式\n");
	else 
		printf("大端模式\n");
#endif
	return 0;
}
