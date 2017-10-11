#include <stdio.h>

/************************************************************************************************
*选择排序法:从一个数组中找到最小的元素然后将其和第一个元素交换，这样实现第一个元素是最小的，
*然后第二次就是找出次小的元素和第二个元素交换，依次进行，直到交换到最后一个元素，整个排序过程完成
*************************************************************************************************/

#if 0
static void choose_select(int buf[], int len)
{
	int i, j, min;
	int tmp = 0;

	for (i = 0; i < len; i++) {
		min = i;//首先把数组的第一个数值认为是最小的
		for (j = i+1; j < len; j++) {
			if (buf[j] < buf[min])
				min = j; //如果a[j]小于a[min],则下标赋值为j
		}

		//将最小的值进行交换
		tmp = buf[i];
		buf[i] = buf[min];
		buf[min] = tmp;
	}
}
#else
/*使用宏的方式比较好，可以体会一下*/
#define key(A) (A)
#define less(A, B) (key(A) < key(B)) // 宏定义一个大小判断的宏
#define exch(A, B) {int t = A; A = B; B = t;}// 交换两个数值大小的宏
#define compexch(A, B) if(less(B, A)) exch(A, B)// 判断大小，并进行交换的宏，如果A大于B，两者进行交换

static void choose_select(int buf[], int len)
{
	int i, j, min;

	for (i = 0; i < len; i++) {
		min = i;//首先把数组的第一个数值认为是最小的
		for (j = i+1; j < len; j++) {
			if (less(buf[j], buf[min]))
				min = j; //如果a[j]小于a[min],则下标赋值为j
		}

		//将最小的值进行交换
		compexch(buf[i], buf[min]);
	}
}

#endif

int main()
{
	int length, i;
	int array[] = {12 ,47 ,478 ,56 ,98 ,33 ,212 ,24, 165, 98};

	length = sizeof(array)/sizeof(int);

	for(i = 0; i < length; i++) {
		printf("%d ", array[i]);
	}

	printf("\n");

	choose_select(array, length);
	
	for(i = 0; i < length; i++) {
		printf("%d ", array[i]);
	}

	printf("\n");
	return 0;
}
