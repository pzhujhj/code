#include <stdio.h>

/************************************************************************************************
*ѡ������:��һ���������ҵ���С��Ԫ��Ȼ����͵�һ��Ԫ�ؽ���������ʵ�ֵ�һ��Ԫ������С�ģ�
*Ȼ��ڶ��ξ����ҳ���С��Ԫ�غ͵ڶ���Ԫ�ؽ��������ν��У�ֱ�����������һ��Ԫ�أ���������������
*************************************************************************************************/

#if 0
static void choose_select(int buf[], int len)
{
	int i, j, min;
	int tmp = 0;

	for (i = 0; i < len; i++) {
		min = i;//���Ȱ�����ĵ�һ����ֵ��Ϊ����С��
		for (j = i+1; j < len; j++) {
			if (buf[j] < buf[min])
				min = j; //���a[j]С��a[min],���±긳ֵΪj
		}

		//����С��ֵ���н���
		tmp = buf[i];
		buf[i] = buf[min];
		buf[min] = tmp;
	}
}
#else
/*ʹ�ú�ķ�ʽ�ȽϺã��������һ��*/
#define key(A) (A)
#define less(A, B) (key(A) < key(B)) // �궨��һ����С�жϵĺ�
#define exch(A, B) {int t = A; A = B; B = t;}// ����������ֵ��С�ĺ�
#define compexch(A, B) if(less(B, A)) exch(A, B)// �жϴ�С�������н����ĺ꣬���A����B�����߽��н���

static void choose_select(int buf[], int len)
{
	int i, j, min;

	for (i = 0; i < len; i++) {
		min = i;//���Ȱ�����ĵ�һ����ֵ��Ϊ����С��
		for (j = i+1; j < len; j++) {
			if (less(buf[j], buf[min]))
				min = j; //���a[j]С��a[min],���±긳ֵΪj
		}

		//����С��ֵ���н���
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
