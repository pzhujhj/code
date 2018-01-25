#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct _test_one {
	int type;
	int len;
}__attribute__((packed)) TEST_ONE;

typedef struct _test_two {
	int type;
	int len;
	char value[0];
}__attribute__((packed)) TEST_TWO;

typedef struct _test_three {
	int type;
	int len;
	char value[1];
}__attribute__((packed)) TEST_THREE;

typedef struct _test_four {
	int type;
	int len;
	char *value;
}__attribute__((packed)) TEST_FOUR;

int main()
{
	TEST_TWO *ttwo;
	TEST_FOUR *tfour;
	int size = 1024;

	/*64位系统指针长度是8,32为系统指针长度是4*/
	printf("one=%lu two=%lu three=%lu four=%lu\n", sizeof(TEST_ONE), sizeof(TEST_TWO), sizeof(TEST_THREE), sizeof(TEST_FOUR));
	
	ttwo = (TEST_TWO *)malloc(sizeof(TEST_TWO) + size);
	ttwo->type = 1;
	ttwo->len = 1;
	strcpy(ttwo->value, "hello");
	printf("two: type=%d len=%d value=%s\n", ttwo->type, ttwo->len, ttwo->value);
	free(ttwo);
	
#if 1
	tfour = (TEST_FOUR *)malloc(sizeof(TEST_FOUR));
	tfour->type = 2;
	tfour->len = 2;
	tfour->value = malloc(sizeof(char)*size);
	strcpy(tfour->value, "aaaaa");
	printf("four: type=%d len=%d value=%s\n", tfour->type, tfour->len, tfour->value);
	printf("%p %p %p\n", &tfour->type, &tfour->len, tfour->value);
	free(tfour->value);
	free(tfour);
#else
	tfour = (TEST_FOUR *)malloc(sizeof(TEST_FOUR) + sizeof(char)*size);
	tfour->type = 2;
	tfour->len = 2;
	tfour->value = ((char *)tfour) + sizeof(TEST_FOUR);
	strcpy(tfour->value, "bbbb");
	printf("four: type=%d len=%d value=%s\n", tfour->type, tfour->len, tfour->value);
	printf("%p %p %p\n", &tfour->type, &tfour->len, tfour->value);
	free(tfour);
#endif	
	
	return 0;
}
