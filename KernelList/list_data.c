#include "list_data.h"
#include "unitil.h"

/********************************************************************************************
* 函数名称:    add_data
* 功能描述:    添加数据到链表中
* 输入参数:    char *data
* 输出参数:    无
* 返回值:      void

********************************************************************************************/
#if 0
void *add_data(char *data, IPLIST_HEAD *p_iplist_head, int length)
{
    IPLIST_DATA *p_iplist_data;
    p_iplist_data = (IPLIST_DATA *)malloc(sizeof(IPLIST_DATA));
    if(p_iplist_data == NULL)
        return NULL;

	memset(p_iplist_data, 0, sizeof(IPLIST_DATA));

    p_iplist_data->data = (char *)malloc(length + 1);
    if(p_iplist_data == NULL)
        return NULL;

    memset(p_iplist_data->data, 0, length+1);

    //IP地址
    memcpy(p_iplist_data->data, data, length);
    //IP长度
    p_iplist_data->len = length;
    //起始时间
    gettimeofday(&p_iplist_data->tv_begin, NULL);
    //初始化标志位
    p_iplist_data->judgeflage = 0;
    
    pthread_rwlock_wrlock(&p_iplist_head->list_rwlock);
    p_iplist_head->count++;
	
    list_add(&p_iplist_data->list_head, &p_iplist_head->list_send_head);
    pthread_rwlock_unlock(&p_iplist_head->list_rwlock);
    
     printf("add new ip data:%d,%s\n",p_iplist_head->count, p_iplist_data->data);
	// free(p_iplist_data->data);
	// free(p_iplist_data);
    return p_iplist_data;
    //return NULL;
}
#endif

#if 1
IPLIST_DATA *add_node(IPLIST_HEAD *p_iplist_head, int length)
{
    IPLIST_DATA *p_iplist_data;
    p_iplist_data = (IPLIST_DATA *)malloc(sizeof(IPLIST_DATA));
    if(p_iplist_data == NULL)
        return NULL;

	memset(p_iplist_data, 0, sizeof(IPLIST_DATA));

    pthread_rwlock_wrlock(&p_iplist_head->list_rwlock);
    list_add(&p_iplist_data->list_head, &p_iplist_head->list_send_head);
    pthread_rwlock_unlock(&p_iplist_head->list_rwlock);
    
    return p_iplist_data;
}
#endif


#if 1
void print_data(struct list_head *head)
{
	struct list_head *list;
	IPLIST_DATA *value;
	
	/*遍历链表不进行节点删除用list_for_each速率较快*/
	//list_for_each(list, &ipshowlist_head->list_send_head) {
	list_for_each(list, head) {
		value = list_entry(list, IPLIST_DATA, list_head);
		printf("print ip data:%s,%d\n", value->data, value->judgeflage);
	}
}
#endif


IPLIST_DATA *find_node(char *data, struct list_head *head)
{
	struct list_head *list;
	IPLIST_DATA *entry;
	
	if((data == NULL) || (strlen(data) == 0)) {
		printf("find data is not empty!\n");
		return NULL;
	}

	list_for_each(list, head) {
		entry = list_entry(list, IPLIST_DATA, list_head);
		if(strcmp(entry->data, data) == 0)
			return entry;
	}

	return NULL;
}


void delete_signal_data(char *data, IPLIST_DATA *deldata, IPLIST_DATA *deltempdata, IPLIST_HEAD *delhead)
{
	if((data == NULL) || (strlen(data) == 0)) {
		printf("del data is not empty!\n");
		return;
	}

	printf("del ip=%s\n", data);
	list_for_each_entry_safe(deldata, deltempdata, &delhead->list_send_head, list_head) {
		if(strcmp(data, deldata->data) == 0) {
			printf("find del data!\n");
			pthread_rwlock_wrlock(&delhead->list_rwlock);
			list_del(&deldata->list_head);
			pthread_rwlock_unlock(&delhead->list_rwlock);
			free(deldata);
			break;
		}
	}

}

void free_list(struct list_head *head, IPLIST_HEAD *freehead)
{
	struct list_head *list;
	IPLIST_DATA *entry;

	printf("---free_list\n");
	list_for_each(list, head) {
		entry = list_entry(list, IPLIST_DATA, list_head);
		pthread_rwlock_wrlock(&freehead->list_rwlock);
		list_del(&entry->list_head);
		pthread_rwlock_unlock(&freehead->list_rwlock);
	}
}

void delete_all_data(IPLIST_DATA *delalldata, IPLIST_DATA *delalltempdata, IPLIST_HEAD *delallhead)
{

	printf("----delall\n");
	list_for_each_entry_safe(delalldata, delalltempdata, &delallhead->list_send_head, list_head) {
		pthread_rwlock_wrlock(&delallhead->list_rwlock);
		list_del(&delalldata->list_head);
		pthread_rwlock_unlock(&delallhead->list_rwlock);
		free(delalldata);
	}
	
}

