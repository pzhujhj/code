#ifndef _UNITIL_H_
#define _UNITIL_H_

#include "list_data.h"
#include "typedef.h"

typedef struct _iplist{
    IPLIST_HEAD *iplist_head;
    IPLIST_DATA *iplist_data,*iplist_temp;

    //线程操作
    pthread_attr_t pattr;	             /** 线程属性 */
    pthread_t p_lgrecv;		            /** 判断当前链表中的数据 */
}IPLIST;

IPLIST *iplist;

void init_list_function(void);
int storage_ip(char *ipAddr, int length);

#endif

