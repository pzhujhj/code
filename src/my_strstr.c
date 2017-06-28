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
	char buf[64] = "abctt45bcdffdbci";
	char *p = NULL;

	p = myStrstr(buf, "df");

	printf("p=%s\n", p);
	return 0;
}
