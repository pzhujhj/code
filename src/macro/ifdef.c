#include <stdio.h>

#define TEST 1

int func1()
{
#if TEST
	printf("#if TEST 1\n");
#else
	printf("#if TEST 0\n");
#endif
	return 0;
}

int func2()
{
#ifdef TEST
	printf("#ifdef TEST 1\n");
#else
	printf("#ifdef TEST 0\n");
#endif
	return 0;
}

int main()
{
	func1();
	func2();
	return 0;
}
