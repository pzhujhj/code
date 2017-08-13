#include <stdio.h>

static void binary_to_assic(unsigned int value)
{
	unsigned int quotient;

	quotient = value/10;
	if (quotient != 0)
		binary_to_assic(quotient);

	putchar(value % 10 + '0');
	//printf("%c", value % 10 + '0');
}

int main()
{
	binary_to_assic(1326);
	return 0;
}
