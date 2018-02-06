#include "typedef.h"
#include "checkApi.h"
#include "unitil.h"

/********************************************************************************************
* 函数名称:    checkip_timethread
* 功能描述:    查找链表中的ip，如果链表中ip对应的时间超过60s，就删除链表中的ip
* 输出参数:    无
* 返回值:      

********************************************************************************************/
void *checkip_timethread(void *arg)
{
	#if 1
    float minux;
    struct  timeval tv_end;
    
    while(1)
    {
    	#if 0
        //printf("===================\n");
        if (iplist->iplist_data) {
	        list_for_each_entry_safe(iplist->iplist_data, iplist->iplist_temp, &iplist->iplist_head->list_send_head, list_head)
	        {
	            gettimeofday(&tv_end,NULL);

	            minux = 1000000*(tv_end.tv_sec-iplist->iplist_data->tv_begin.tv_sec)+tv_end.tv_usec-iplist->iplist_data->tv_begin.tv_usec;                       
	            minux/=1000000;

	            printf("print ip data:%s,%d,%d\n",iplist->iplist_data->data, iplist->iplist_data->judgeflage, iplist->iplist_head->count);
	            
	            //if(minux>=60)
	            //{
	            //    delete_data(p_Root->p_netrecv_data,p_Root->p_netlist_head);
	            //}
	            
	        }
        }
		#endif
		//print_data(iplist->iplist_data, iplist->iplist_head);
        usleep(60000000);//每隔5s中进行一次遍历链表
    }
	#endif
}

