#include<stdio.h>
#include <string.h>

#if 0
static char *my_strcpy(char *dest, const char *src)
{
	if(src == NULL)
		return NULL;
	
	//while((*dest++ = *src++) != '\0');
	while(*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}

	return NULL;
}
#else
static char *my_strcpy(char *dest, const char *src)
{
	char *address = NULL;

	if(src == NULL)
		return NULL;

	address = dest;
	
	while((*dest++ = *src++) != '\0');
//	while(*src != '\0') {
//		*dest = *src;
//		dest++;
//		src++;
//	}

	return address;
}
#endif

int main()
{
	char buf[] = "fadfdtaeef";
	char msg[64] = {0};

	memset(msg, 0, 64);
	my_strcpy(msg, buf);
	
	printf("msg=%s\n", msg);

	return 0;
}
