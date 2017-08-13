#include <stdio.h>
#include <string.h>

char *myStrstr(char const *src, char const *target)
{
	register char *last = NULL;
	register char *current = NULL;

	if (*target != '\0') {
		//查找target在src中第一次出现的位置
		current = strstr(src, target);
	
		while(current != NULL) {
			last = current;
			current = strstr(last+1, target);
		}
	}

	return  last;
}

int main()
{
	char buf[64] = "gz_test:id:7000028";
	char *p = NULL;

	p = myStrstr(buf, ":");//返回的肯能是一个空值，所以在使用p的时候需要做判断

	if (p)
		printf("p=%s\n", p+1);
	return 0;
}
