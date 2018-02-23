#include <stdio.h>
#include <string.h>

/*实现字符串:AkleBiCeilD,排序后为:kleieilABCD*/

#define MAX_SIZE 100

int main() 
{
	char upper[MAX_SIZE] =  {0};
	char lower[MAX_SIZE] = {0};
	char data[MAX_SIZE] = {0};
	int len = 0, i;
	char *s_lower, *s_upper;

	s_lower = lower;
	s_upper = upper;

	printf("please input str:\n");
	while((scanf("%s", data)) != EOF) {
		memset(upper, 0, MAX_SIZE);
		memset(lower, 0, MAX_SIZE);
		
		len = strlen(data)>MAX_SIZE?MAX_SIZE:strlen(data);
		for (i = 0; i < len; i++) {
			if(data[i] > 'a' && data[i] < 'z') {
				*s_lower++ = data[i];
			}else {
				*s_upper++ = data[i];
			}
		}

		len = strlen(lower);
		strncpy(data, lower, len);
		strncpy(data+len, upper, strlen(upper));

		printf("data=%s\n", data);
		printf("please input str:\n");
		memset(data, 0, MAX_SIZE);
	}

	
	return 0;
}
