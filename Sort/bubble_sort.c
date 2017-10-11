#include <stdio.h>

/************************************************************************************************
*基本思想： 不断比较相邻的两个数，让较大的数不断地往后移。经过一番比较，就选出了最大的数。
*经过第二轮比较，就选出了次大的数。以此类推。 那么对于大小为N的数组，需要N-1轮比较。
*************************************************************************************************/
/*使用宏的方式比较好，可以体会一下*/
#define key(A) (A)
#define less(A, B) (key(A) < key(B)) // 宏定义一个大小判断的宏
#define exch(A, B) {int t = A; A = B; B = t;}// 交换两个数值大小的宏
#define compexch(A, B) if(less(B, A)) exch(A, B)// 判断大小，并进行交换的宏，如果A大于B，两者进行交换

static void bubble_sort(int buf[], int len)
{
	int i, j;

	for (i = 0; i < len; i++) {
		for (j = i+1; j < len; j++) {
			if (less(buf[j], buf[i]))
				compexch(buf[i], buf[j]);
		}
		
	}
}

int main()
{
	int length, i;
	int array[] = {12 ,47 ,478 ,56 ,98 ,33 ,212 ,24, 165, 98};

	length = sizeof(array)/sizeof(int);

	for(i = 0; i < length; i++) {
		printf("%d ", array[i]);
	}

	printf("\n");

	bubble_sort(array, length);
	
	for(i = 0; i < length; i++) {
		printf("%d ", array[i]);
	}

	printf("\n");
	return 0;
}
