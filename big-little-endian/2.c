#include <stdio.h>

/*大端模式：内存高地址存放低字节，内存低地址存放高字节
 *
 *小端模式：内存高地址存放高字节，内存低地址存放低字节
 * */
int main()
{
	int i;
	int m = 0x12345678;

	printf("m=%d\n", m);
	char *p = (char *)&m;

	printf("%d\n", sizeof(m));
	for (i = 0; i < sizeof(m); i++) {
		printf("%x ", *p++); //内存地址在增加
	}
	printf("\n");

	if (*(p-1) == 0x78)
		printf("大端模式!\n");
	else if (*(p-1) == 0x12)
		printf("小端模式!\n");

	return 0;
}
