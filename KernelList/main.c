#include <stdio.h>
#include "unitil.h"
#include "list_data.h"

int main()
{
    //char inputdata[20] = "1.1.1.1";
	char inputdata[20] = {0};
	IPLIST_DATA *entry;

	init_list_function();
#if 0 
	for(;;) {
		resultvalue = storage_ip(inputdata, strlen(inputdata));
		print_data(&iplist->iplist_head->list_send_head);
		//delete_signal_data(inputdata, iplist->iplist_data, iplist->iplist_temp, iplist->iplist_head);
		//free_list(&iplist->iplist_head->list_send_head);
		//print_data(&iplist->iplist_head->list_send_head);
	}

#endif
	

#if 1
  //  while(1)
  //  {
        memset(inputdata, 0, sizeof(inputdata));
		printf("please intput flag:\n");
		scanf("%s",inputdata);

		if (!strcmp(inputdata, "del")) {
			printf("please input del ip:\n");
			scanf("%s",inputdata);
			delete_signal_data(inputdata, iplist->iplist_data, iplist->iplist_temp, iplist->iplist_head);
		}else if (!strcmp(inputdata, "show")) {
			print_data(&iplist->iplist_head->list_send_head);
		}
		else if(!strcmp(inputdata, "add")) {
			printf("please input ip:\n");
			scanf("%s",inputdata);
			storage_ip(inputdata, strlen(inputdata));
		}else if (!strcmp(inputdata, "delall")) {
			free_list(&iplist->iplist_head->list_send_head, iplist->iplist_head);
			//delete_all_data(iplist->iplist_data, iplist->iplist_temp, iplist->iplist_head);
		}else if (!strcmp(inputdata, "find")) {
			printf("please input ip:\n");
			scanf("%s",inputdata);
			entry = find_node(inputdata, &iplist->iplist_head->list_send_head);
			if(entry)
				printf("%s-%d-%d\n", entry->data, entry->len, entry->judgeflage);
			else
				printf("find fail!\n");
		}
   // }
#endif

	free(iplist->iplist_head);
	free(iplist);
    return 0;
}



