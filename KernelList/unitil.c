#include "unitil.h"
#include "checkApi.h"

/********************************************************************************************
* 函数名称:    init_list_function
* 功能描述:    初始化链表
* 输入参数:无   
* 输出参数:    无
* 返回值:      

********************************************************************************************/
void init_list_function(void)
{
	int err = -1;

	iplist = (IPLIST *) malloc(sizeof(IPLIST));
	memset(iplist, 0, sizeof(IPLIST));

	iplist->iplist_data = NULL;
	iplist->iplist_temp = NULL;

	/*设置线程属性,暂且为默认属性*/

	/*创建线程*/
	err = pthread_create(&iplist->p_lgrecv, NULL, (void *)checkip_timethread, NULL);
	if(err != 0) {
		perror("pthread create error!\n");
		return;
	}

	iplist->iplist_head = (IPLIST_HEAD *)malloc(sizeof(IPLIST_HEAD));
	memset(iplist->iplist_head, 0, sizeof(IPLIST_HEAD));

	/** 初始化读写锁 */
	pthread_rwlock_init(&iplist->iplist_head->list_rwlock, NULL);

	INIT_LIST_HEAD(&iplist->iplist_head->list_send_head);
}

#if 1
/********************************************************************************************
* 函数名称:    storage_ip
* 功能描述:    将ip存储在链表中，新加入的ip地址与链表的IP进行比较,如果ip存在就将标志设为1，如果不存在就添加
			ip到链表中，同时标志设为0
* 输入参数:    char *ipAddr   int length    
* 输出参数:    无
* 返回值:      

********************************************************************************************/
int storage_ip(char *ipAddr, int length)
{
	int flag = 0;
	IPLIST_DATA *entry;
    
    if(length < 7) {
		printf("input ip is not right!\n");
		return -1;
    }

	#if 1
	list_for_each_entry_safe(iplist->iplist_data, iplist->iplist_temp, &iplist->iplist_head->list_send_head, list_head) {
		if(!strcmp(iplist->iplist_data->data, ipAddr)) {
			iplist->iplist_data->judgeflage = 1;
			flag = 1;
			break;
		}
	}
	#endif
	
	if(flag == 0)
		entry = add_node(iplist->iplist_head, length);

//	if(flag == 0)
//		entry = add_node(iplist->iplist_head, length, iplist->iplist_data);

	if (entry) {
	    //IP地址
	    memcpy(entry->data, ipAddr, length);
	    //IP长度
	    entry->len = length;
	    //起始时间
	    gettimeofday(&entry->tv_begin, NULL);
	    //初始化标志位
	    entry->judgeflage = 0;
	}
	
	iplist->iplist_head->count++;

	return 0;
}
#endif
