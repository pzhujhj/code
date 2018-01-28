#include <stdio.h>

/******************
*实现二分法查找
******************/

#if 0
//method one
static int binsearch(int *array, int low, int hight, int value)
{
	int mid = -1;
	
	while(low <= hight) {
		mid = low + (hight - low)/2;
		if(array[mid] > value)
			hight = mid - 1;
		else if (array[mid] < value)
			low = mid + 1;
		else 
			return mid;
	}
	
	return -1;
}
#else

//method two
static int binsearch(int *array, int low, int hight, int value)
{
	int mid = -1;

	mid = low + (hight - low)/2;
	if (low <= hight) {
		if(array[mid] > value)
			return binsearch(array, low, mid-1, value);
		else if (array[mid] < value)
			return binsearch(array, mid+1, hight, value);
		else 
			return mid;	
	}
	return -1;
}
#endif

int main()
{
	int ret;
	int buf[12] = {1,4,7,9,11,12,31,33,66,72,75,77};

	ret = binsearch(buf, 0, sizeof(buf)/sizeof(int)-1, 11);
	printf("ret=%d\n", ret);

	return 0;
}
