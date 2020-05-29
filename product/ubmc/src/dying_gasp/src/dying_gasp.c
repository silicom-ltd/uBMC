/*
 * dying_gasp.c
 *
 *  Created on: Jul 20, 2018
 *      Author: jason_zhang
 */

#include "dying_gasp.h"

/* creat a simple trap session */
netsnmp_session* slic_snmp_create_session(char *peername,char *com, int version)
{
	netsnmp_session session;
	netsnmp_session *ssp;
	ssp = malloc(sizeof(netsnmp_session));
	if( ssp == NULL)
	{
		//printf("%s %s err",__function__,__line__);
		return NULL ;
	}
	snmp_sess_init(&session);
	session.version = version;
	session.peername = strdup(peername);
	session.community = strdup(com);
	session.community_len = strlen(com);
	ssp = snmp_open(&session);
	if(ssp == NULL)
	{
		DG_DEBUG_ERR("creat session is fail \n");
	}
	return ssp;
}

/* just send a trap  */
int silc_snmp_send_simple_trap(char* trap_name, char* module, int event,netsnmp_session* session)
{
	const oid  objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };
	const oid  objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
    const oid snmptrap_oid[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
	const oid trapPower_oid[] = { 1,3,6,1,4,1,15694,4,11,4 };

	//const oid trapModule_oid[] = { 1, 3, 6, 1, 4, 1, 15694, 4, 5, 0, 1, 0, 1 };
	//const oid trapEvent_oid[] = { 1, 3, 6, 1, 4, 1, 15694, 4, 5, 0, 1, 0, 2 };
    const oid trapDyinggasp_oid[] = { 1,3,6,1,4,1,15694,5,11,8 };
    const oid trapModule_oid[] = { 1, 3, 6, 1, 4, 1, 15694, 5, 11, 0, 1 };
    const oid trapEvent_oid[] = { 1, 3, 6, 1, 4, 1, 15694, 5, 11, 0, 2 };

	long            sysuptime;
	char            csysuptime[20];
	char *timeup;
	int ret;
	netsnmp_pdu    *pdu;
	pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
	if ( !pdu )
	{
		fprintf(stderr, "Failed to create notification PDU\n");
		return -1;
		//exit(1);
	}
	timeup = csysuptime;
	sysuptime = get_uptime();
	sprintf(csysuptime, "%ld", sysuptime);
	snmp_pdu_add_variable(pdu, objid_sysuptime,OID_LENGTH(objid_sysuptime), ASN_TIMETICKS, timeup,sizeof(timeup));
	snmp_pdu_add_variable(pdu,snmptrap_oid,OID_LENGTH(snmptrap_oid),ASN_OBJECT_ID,trapDyinggasp_oid,sizeof(trapDyinggasp_oid));
	snmp_pdu_add_variable(pdu,snmptrap_oid,OID_LENGTH(snmptrap_oid),ASN_OBJECT_ID,trapPower_oid,sizeof(trapPower_oid));
	snmp_pdu_add_variable(pdu,trapModule_oid,OID_LENGTH(trapModule_oid),ASN_OCTET_STR,module,strlen(module));
	snmp_pdu_add_variable(pdu,trapEvent_oid,OID_LENGTH(trapEvent_oid),ASN_INTEGER, &event,sizeof(event));
	ret = snmp_send(session, pdu);
	return ret;
}

#define PID_BUF_SIZE 42
/*listen some process's pid which we need to kill */
int listen_pid_change(struct pids_need_to_kill_s* pid,const char* cmd)
{
	char pid_buf[PID_BUF_SIZE] = {0};
	char *endptr;
	int pidd = 0,ret = 0;
	int status;
	char cmd_buf[CMD_BUF_SIZE] = {0};
	char tmp_buf[30] = {0};
	int offset;
	FILE *fp = popen(cmd,"r");
	while(NULL != fgets(pid_buf,10,fp) )
	{
		pidd = strtol(pid_buf,&endptr,0);
		DG_DEBUG_INFO("pid is %d \n",pidd);
		pid->pids[ret] = pidd;
		sprintf(tmp_buf,"kill -9 %d;",pid->pids[ret]);
		strcpy(cmd_buf + offset,tmp_buf);
		offset = offset + strlen(tmp_buf);
		memset(tmp_buf,'\0',sizeof(tmp_buf));
		ret ++;
	}

	//pthread_mutex_lock(&(pid->lock));
	memset(pid->kill_cmd,'\0',CMD_BUF_SIZE);
	strcpy(pid->kill_cmd,cmd_buf);
	//pthread_mutex_unlock(&(pid->lock));
	DG_DEBUG_INFO("cmd is :%s \n",pid->kill_cmd);
	pclose(fp);
	pid->pid_num = ret;
	return ret;

}

int mgmtd_client_config_check(const silc_cstr path ,silc_mgmtd_if_req_type type, const silc_cstr node_root_val_str)
{
        int ret = 0;
        silc_mgmtd_msg* p_msg;
        silc_mgmtd_if_req *p_req;
        silc_mgmtd_if_rsp *p_rsp;
        p_req = silc_mgmtd_if_req_create(path, type, node_root_val_str);
        if(!p_req)
            return -1;

        p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
        if(!p_msg)
        {
            silc_mgmtd_if_req_delete(p_req);
            return -1;
        }
        p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
        if(!silc_mgmtd_if_rsp_check(p_rsp))
        {
            is_err("Query the console manager bmc configuration failed: %s.\n", p_rsp->return_str);
            ret = -1;
        }
        else
        {
            ret = 0;
        }
        silc_mgmtd_if_req_delete(p_req);
        silc_mgmtd_if_client_free_msg(p_msg);
        return ret;

}

int init_config_change_callback(const silc_cstr cfg_changed_path,silc_mgmtd_if_notify_cb daemon_entry,void *data)
{
	int ret;
	ret = silc_mgmtd_if_client_set_recv_notify(cfg_changed_path,daemon_entry,data);

    silc_mgmtd_if_client_start_notify_daemon();
	return ret;
}

void clear_trap_table(trap_hosts_table *table)
{
	memset(table->trap_table,'0',sizeof(trap_table)*TRAP_TABLE_MAXSIZE);
	table->trap_table_num = 0;

}
/*update the host trap table */
int update_trap_table(trap_hosts_table *table)
{
    silc_mgmtd_msg* p_msg;
    silc_mgmtd_if_req *p_req;
    silc_mgmtd_if_rsp *p_rsp;
    char *ptr_end;
    int ret;
    int cnt = 0;
   // clear_trap_table(table);
    p_req = silc_mgmtd_if_req_create("/config/snmp/agent", SILC_MGMTD_IF_REQ_QUERY_SUB, "");
    if(!p_req)
    {
    	DG_DEBUG_ERR("silc_mgmtd_if_req_create is fail \n");
    	return -1;
    }
    p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
    if(!p_msg)
    {
        silc_mgmtd_if_req_delete(p_req);
        return -1;
    }
    p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
    if(!silc_mgmtd_if_rsp_check(p_rsp))
    {
          is_err("Query the console manager bmc configuration failed: %s.\n", p_rsp->return_str);
          return -1;
    }
     else
     {
    	 silc_mgmtd_if_node *p_node;
    	 silc_mgmtd_if_node *p_mod_sub_node;
    	 silc_mgmtd_if_node *p_ip_sub_node;
    	 bool temp_bool;
    	 unsigned int temp_u32;
    	 unsigned int len;
    	 // get trap-host node and get trap server information
    	 silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
    	 {
    	   DG_DEBUG_INFO("p_node->val_str is %s \n",p_node->name);
    	   if(strcmp(p_node->name, "trap-hosts") == 0)
    	   {
    		   //printf("trap-hosts \n");
    		   silc_list_for_each_entry(p_mod_sub_node, &p_node->sub_node_list, node)
			   {
    			   //printf("p_mod_sub_node : %s \n",p_mod_sub_node->name);
    			   if(p_mod_sub_node->name !=NULL)
    			   {
    				  // printf("cnt is %d p_mod_sub_node->val_str is %s \n",cnt,p_mod_sub_node->val_str);
					   strcpy(table->trap_table[cnt]->ip,p_mod_sub_node->val_str);
    				   silc_list_for_each_entry(p_ip_sub_node, &p_mod_sub_node->sub_node_list, node)
					   {
    					   //printf("p_ip_sub_node : %s \n",p_ip_sub_node->name);
    					   if(strcmp(p_ip_sub_node->name, "version") == 0)
    					   {
    						   if(p_ip_sub_node->val_str != NULL)
    						   {
    							   strcpy(table->trap_table[cnt]->version,p_ip_sub_node->val_str);
    							   //printf("tab version :%s  \n",table->trap_table[cnt]->vesion);
    						   }
    					   }
    					   if(strcmp(p_ip_sub_node->name, "community") == 0)
    					   {
    						   if(p_ip_sub_node->val_str != NULL)
    						   {
    						    // memcpy(table[cnt]->community,p_ip_sub_node->val_str,len =sizeof(table[cnt]->community) < strlen(p_ip_sub_node->val_str)?sizeof(table[cnt]->community):strlen(p_ip_sub_node->val_str));// sizeof(table[cnt]->community) < sizeof(p_ip_sub_node->val_str)?sizeof(table[cnt]->community) : sizeof(p_ip_sub_node->val_str));
    							   strcpy(table->trap_table[cnt]->community,p_ip_sub_node->val_str);
    						   }
    					       //printf("tab:community is %s \n",table->trap_table[cnt]->community);
    					   }

    					   if(strcmp(p_ip_sub_node->name, "port") == 0)
    					   {
    						   if(p_ip_sub_node->val_str != NULL)
    						   {

								   temp_u32 = strtol(p_ip_sub_node->val_str,&ptr_end,0);
								   table->trap_table[cnt]->port = temp_u32;
        						  // printf("tab port is %d \n",table->trap_table[cnt]->port);
    						   }
    					   }
					   }
    				   cnt++;
    				   if(cnt > TRAP_TABLE_MAXSIZE - 1)
    				   {
    					   cnt = TRAP_TABLE_MAXSIZE - 1;
    					   DYING_GASP_LOG("The number of trap host is too large for dying gasp\n");
    				   }

    			   }
    			   else
    			   {
    				   DG_DEBUG_ERR("there is no sub node \n");
    				   return -1;
    			   }
    			   table->trap_table_num = cnt;
    			  //printf("table_num is %d \n",table->trap_table_num);
			   }
    	   }
    	   else
    	   {
    		   //DG_DEBUG_INFO("find no trap-hosts node \n");
    		  // return -1;
    	   }
    	 }
     }
     silc_mgmtd_if_req_delete(p_req);
     silc_mgmtd_if_client_free_msg(p_msg);
     return 0;
}



int update_snmp_session_table(trap_hosts_table *table)
{
	int session_num;
	int i;
	char peer_buf[PEERNAME_SIZE];
	session_num = table->trap_table_num;
	for(i = 0;i <session_num ;i ++)
	{
		sprintf(peer_buf,"%s:%d",table->trap_table[i]->ip,table->trap_table[i]->port);
		strcpy(table->session[i]->peername,peer_buf);
		strcpy(table->session[i]->community,table->trap_table[i]->community);
		//strcpy(table->session[i]->version,table->trap_table[i]->version);
	}
	return 0;
}


int init_trap_host_table(trap_hosts_table *table)
{
	int i;
	DG_DEBUG_INFO("init_trap_host_table start \n");
	for(i = 0; i < TRAP_TABLE_MAXSIZE; i ++)
	{
		table->trap_table[i]= malloc(sizeof(trap_table));
		if(table->trap_table[i] == NULL)
		{
			DG_DEBUG_ERR("init_trap_host_table  trap_table fail \n");
			return -1;
		}
		table->session[i] = malloc(sizeof(netsnmp_session));
		if(table->session[i] == NULL)
		{
			DG_DEBUG_ERR("init_trap_host_table session fail \n");
			return -1;
		}
		table->session[i]->community = malloc(sizeof(char)*COMMUNITY_SIZE);
		table->session[i]->peername = malloc(sizeof(char)*PEERNAME_SIZE);

	}
	table->trap_table_num = 0;
	DG_DEBUG_INFO("init_trap_host_table done \n");
	return 0;
}

