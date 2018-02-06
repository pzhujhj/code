#ifndef _LIST_DATA_H_
#define _LIST_DATA_H_
#include "typedef.h"
#include "list.h"


/*定义链表头*/
typedef struct _iplist_head{
    int count;                                    //当前数据个数
    pthread_rwlock_t list_rwlock;                 //读写操作锁
    struct list_head list_send_head;
}IPLIST_HEAD;

/** 发送数据结构体 */
typedef struct _iplist_data{
	int judgeflage; 												//标志位
	int len;                                                        //IP长度
    char data[32];                                                    //发送的数据
    struct  timeval tv_begin;                            //IP第一次收到的时间    
    struct list_head list_head;
	char test[2048];
}IPLIST_DATA;

//void *add_data(char *data, IPLIST_HEAD *p_iplist_head, int length);
IPLIST_DATA *add_node(IPLIST_HEAD *p_iplist_head, int length);
//IPLIST_DATA *add_node(IPLIST_HEAD *p_iplist_head, int length, IPLIST_DATA *p_iplist_data);

void print_data(struct list_head *head);
void delete_signal_data(char *data, IPLIST_DATA *deldata, IPLIST_DATA *deltempdata, IPLIST_HEAD *delhead);
void delete_all_data(IPLIST_DATA *delalldata, IPLIST_DATA *delalltempdata, IPLIST_HEAD *delallhead);
IPLIST_DATA *find_node(char *data, struct list_head *head);
void free_list(struct list_head *head, IPLIST_HEAD *freehead);


#endif
