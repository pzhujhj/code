#include <stdio.h>

#define TRUE 1
#define FALSE 0

typedef unsigned char bool;

bool testfun()
{
	return TRUE;	
}

int main()
{
	bool ret;

	ret = testfun();
	printf("ret=%d\n", ret);
	return 0;
}
