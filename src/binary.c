#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static unsigned char tmp = 0;

/*int m=9  binary=0000 0000 0000 0000 0000 0000 0000 1001
 *num<<i)>>(len-1):
 *i=1   0000 0000 0000 0000 0000 0000 0001 0010 >> 31 ---> 0000 0000 0000 0000 0000 0000 0000 0000
 *i=28  1001 0000 0000 0000 0000 0000 0000 0000 >> 31 ---> 0000 0000 0000 0000 0000 0000 0000 0001
 * */

#if 0
//这种方式len = sizeof(int)*8不能换成len = sizeof(char)*8
static void showBinary(int num)
{
	int i;
	int len;

	len = sizeof(int)*8;
	for (i = 0; i < len; i++)
		printf("%d",((unsigned)(num<<i)>>(len-1))); 
//		putchar('0'+((unsigned)(num<<i)>>(len-1)));

	printf("\n");
}
#else
//这种方式len = sizeof(int)*8能换成len = sizeof(char)*8
//static void showBinary(int num)
static void showBinary(unsigned char num)
{
	int i;
	int len;

	//len = sizeof(int)*8;
	len = sizeof(char)*8;
	for (i = 0; i < len; i++) {
		printf("%d",((unsigned)(num&(1<<(len-1)))>>(len-1))); //这步是对num的高位进行处理，然后输出该位二进制 
//		putchar('0'+((unsigned)(num&(1<<(len-1)))>>(len-1)));
		num <<= 1;
	}
	printf("\n");
}
#endif

static void func(int value)
{
	static int bit = 0;

	if (value == 0) {
		//tmp &= 0b01111111;
		tmp &= 0x7f; //如果传输过来的数据为0，&=0x7f会保证该位的值为0
	}else {
		//tmp |= ~0b01111111;
		tmp |= 0x80; //如果传输过来的数据为1，|=0x80会保证该位的值为1
	}
	
	if (bit != 7)
		tmp >>= 1;

	bit++;
}

int main()
{
#if 0
	/*0x55  01010101 
	 *串口通信协议规定数据位是从地位开始传送的，所以传输顺序：1->0->1->0->1->0->1->0
	 * */
	int i;
	for (i = 1; i <= 8; i++) {
		if (i%2 == 0)
			func(0);
		else 
			func(1);
	}
#else
	/*0x23 00100011    传输顺序: 1->1->0->0->0->1->0->0*/
	func(1);
	func(1);
	func(0);
	func(0);
	func(0);
	func(1);
	func(0);
	func(0);
#endif

	showBinary(tmp);
	printf("tmp=%x %d\n", tmp, tmp);

	return 0;
}
