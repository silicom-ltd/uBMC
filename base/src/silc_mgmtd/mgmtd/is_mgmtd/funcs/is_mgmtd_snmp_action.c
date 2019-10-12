/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "is_mgmtd_func_def.h"

#ifdef SILC_MGMTD_LOCAL_DEBUG
#define SILC_SNMP_CONF_FULL_NAME    "/etc/snmp/snmpd.conf"
#define SILC_SNMP_CONF2_FULL_NAME 	"/var/lib/net-snmp/snmpd.conf"
#else
#define SILC_SNMP_CONF_FULL_NAME    "/etc/snmp/snmpd.conf"
#define SILC_SNMP_CONF2_FULL_NAME 	"/var/lib/snmp/snmpd.conf"
#endif

#define SILC_SNMP_CONF_BUFF_MAX	4096
#define SILC_SNMP_VERSION_V1	1
#define SILC_SNMP_VERSION_V2C	2
#define SILC_SNMP_VERSION_V3	3

static int snmp_action_add_agent_communities(silc_cstr conf_buff, silc_mgmtd_node* p_commu_node)
{
	silc_mgmtd_node *p_node, *p_sub_node;

	silc_list_for_each_entry(p_node, &p_commu_node->sub_node_list, node)
	{
		silc_cstr name = NULL, access = NULL;
		silc_bool state = silc_false;

		if(p_node->type == MEMDB_NODE_TYPE_TEMPLATE)
			continue;
		name = p_node->value.val.string_val;
		silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
		{
			if(strcmp(p_sub_node->name, "full-access") == 0)
			{
				if(p_sub_node->value.val.bool_val)
					access = "rwcommunity";
				else
					access = "rocommunity";
			}
			else if(strcmp(p_sub_node->name, "state") == 0)
				state = p_sub_node->value.val.bool_val;
		}
		if(!name || !access)
			return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

		if(!state)
			continue;

		sprintf(conf_buff+strlen(conf_buff), "%s %s\n", access, name);
	}
	return 0;
}

static int snmp_action_add_agent_users(silc_cstr conf_buff, silc_cstr conf2_buff, silc_mgmtd_node* p_user_node)
{
	silc_mgmtd_node *p_node, *p_sub_node;

	silc_list_for_each_entry(p_node, &p_user_node->sub_node_list, node)
	{
		silc_cstr name = NULL, access = NULL, passwd = NULL, auth = NULL;
		silc_bool state = silc_false;

		if(p_node->type == MEMDB_NODE_TYPE_TEMPLATE)
			continue;
		name = p_node->value.val.string_val;
		silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
		{
			if(strcmp(p_sub_node->name, "full-access") == 0)
			{
				if(p_sub_node->value.val.bool_val)
					access = "rwuser";
				else
					access = "rouser";
			}
			else if(strcmp(p_sub_node->name, "auth") == 0)
				auth = p_sub_node->value.val.string_val;
			else if(strcmp(p_sub_node->name, "state") == 0)
				state = p_sub_node->value.val.bool_val;
			else if(strcmp(p_sub_node->name, "password") == 0)
				passwd = p_sub_node->value.val.string_val;
		}
		if(!name || !access || !passwd || !auth)
			return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

		if(!state)
			continue;

		sprintf(conf_buff+strlen(conf_buff), "%s %s\n", access, name);
		if(strcmp(auth, "sha") == 0)
			sprintf(conf2_buff+strlen(conf2_buff), "createUser %s SHA \"%s\" AES %s\n", name, passwd, passwd);
		else if(strcmp(auth, "md5") == 0)
			sprintf(conf2_buff+strlen(conf2_buff), "createUser %s MD5 \"%s\" DES %s\n", name, passwd, passwd);
	}
	return 0;
}

static int snmp_action_add_trap_hosts(silc_cstr conf_buff, silc_mgmtd_node* p_trap_host)
{
	silc_mgmtd_node *p_node, *p_sub_node;

	silc_list_for_each_entry(p_node, &p_trap_host->sub_node_list, node)
	{
		char ip[64];
		silc_cstr commu = NULL, passwd = NULL, auth = NULL, ver = NULL;
		silc_bool state = silc_false;

		if(p_node->type == MEMDB_NODE_TYPE_TEMPLATE)
			continue;
		silc_mgmtd_var_to_str(&p_node->value, ip, 64);
		silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
		{
			if(strcmp(p_sub_node->name, "version") == 0)
			{
				ver = p_sub_node->value.val.string_val;
			}
			else if(strcmp(p_sub_node->name, "state") == 0)
				state = p_sub_node->value.val.bool_val;
			else if(strcmp(p_sub_node->name, "community") == 0)
				commu = p_sub_node->value.val.string_val;
			else if(strcmp(p_sub_node->name, "auth") == 0)
				auth = p_sub_node->value.val.string_val;
			else if(strcmp(p_sub_node->name, "password") == 0)
				passwd = p_sub_node->value.val.string_val;
		}

		if(!ver)
			return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

		if(!state)
			continue;

		if(strcmp(ver, "v1") == 0)
		{
			if(!commu)
				return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
			sprintf(conf_buff+strlen(conf_buff), "trapsink %s %s\n", ip, commu);
		}
		else if(strcmp(ver, "v2c") == 0)
		{
			if(!commu)
				return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
			sprintf(conf_buff+strlen(conf_buff), "trap2sink %s %s\n", ip, commu);
		}
		else if(strcmp(ver, "v3") == 0)
		{
			if(!commu || !auth || !passwd)
				return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
			if(strcmp(auth, "sha") == 0)
				sprintf(conf_buff+strlen(conf_buff), "trapsess -v 3 -Ci -a SHA -A %s -x AES -X %s -l authPriv -u %s %s\n",
					passwd, passwd, commu, ip);
			else if(strcmp(auth, "md5") == 0)
				sprintf(conf_buff+strlen(conf_buff), "trapsess -v 3 -Ci -a MD5 -A %s -x DES -X %s -l authPriv -u %s %s\n",
					passwd, passwd, commu, ip);
			else
				return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
		}
		else
			return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
	}
	return 0;
}

static int snmp_action_add_sysoid(silc_cstr conf_buff)
{
	silc_cstr sysoid = NULL;

	if(silc_mgmtd_memdb_get_product_info()->get_snmp_sysoid_func)
		sysoid = silc_mgmtd_memdb_get_product_info()->get_snmp_sysoid_func();

	if(sysoid)
		sprintf(conf_buff+strlen(conf_buff), "sysObjectID %s\n", sysoid);

	return 0;
}

static int snmp_action_stop_snmpd(void)
{
	int ret = 0;
#ifdef SILC_MGMTD_LOCAL_DEBUG
	ret = silc_mgmtd_if_exec_system_cmd("sudo service snmpd stop", NULL, NULL, 10000, silc_false);
#else
	ret = silc_mgmtd_if_exec_system_cmd("killall -q -9 snmpd", NULL, NULL, 10000, silc_false);
#endif
	SILC_LOG("[%s] snmp stop, ret:%d\n", __func__, ret);
	return ret;
}

int snmp_action_start_snmpd(void)
{
	int ret = 0;

#ifdef SILC_MGMTD_LOCAL_DEBUG
	ret = silc_mgmtd_if_exec_system_cmd("sudo service snmpd start", NULL, NULL, 10000, silc_false);
#else
	ret = silc_mgmtd_if_exec_system_cmd("snmpd &", NULL, NULL, 10000, silc_false);
#endif
	SILC_LOG("[%s] snmp start, ret:%d\n", __func__, ret);
	return ret;
}

static int snmp_action_apply_to_snmpd_conf(void)
{
	int ret = 0;
	silc_cstr conf_str = NULL, conf2_str = NULL;
	silc_mgmtd_node *p_snmp_state, *p_commu, *p_user, *p_traphost;

	p_snmp_state = silc_mgmtd_memdb_find_node("/config/snmp/agent/state");
	p_commu = silc_mgmtd_memdb_find_node("/config/snmp/agent/communities");
	p_user = silc_mgmtd_memdb_find_node("/config/snmp/agent/users");
	p_traphost = silc_mgmtd_memdb_find_node("/config/snmp/agent/trap-hosts");

	if(!p_snmp_state || !p_commu || !p_user || !p_traphost)
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

	if(p_snmp_state->value.val.bool_val == silc_false)
	{
		snmp_action_stop_snmpd();
		return 0;
	}

	conf_str = malloc(SILC_SNMP_CONF_BUFF_MAX);
	conf2_str = malloc(SILC_SNMP_CONF_BUFF_MAX);
	if(!conf_str || !conf2_str)
	{
		ret = 2;
		goto OUT;
	}
#ifdef SILC_MGMTD_LOCAL_DEBUG
	sprintf(conf_str, "sudo echo '''master agentx\nagentAddress udp:161\n");
	sprintf(conf2_str, "sudo echo '''engineBoots 1\n");
#else
	sprintf(conf_str, "echo '''master agentx\nagentAddress udp:161\n");
	sprintf(conf2_str, "echo '''engineBoots 1\n");
#endif

	ret = snmp_action_add_agent_communities(conf_str, p_commu);
	if(ret != 0)
		goto OUT;

	ret = snmp_action_add_agent_users(conf_str, conf2_str, p_user);
	if(ret != 0)
		goto OUT;

	ret = snmp_action_add_trap_hosts(conf_str, p_traphost);
	if(ret != 0)
		goto OUT;

	ret = snmp_action_add_sysoid(conf_str);
	if(ret != 0)
		goto OUT;

	sprintf(conf_str+strlen(conf_str), "''' > %s", SILC_SNMP_CONF_FULL_NAME);
	sprintf(conf2_str+strlen(conf2_str), "''' > %s", SILC_SNMP_CONF2_FULL_NAME);

//	printf("snmpd.conf:\n%s\n", conf_str);
//	printf("var snmpd.conf:\n%s\n", conf2_str);

	snmp_action_stop_snmpd();
	usleep(500000);
#ifndef SILC_MGMTD_LOCAL_DEBUG
	system("mkdir -p /etc/snmp");
	system("mkdir -p /var/lib/snmp");
#endif
	system(conf_str);
	system(conf2_str);
	usleep(500000);
	snmp_action_start_snmpd();
OUT:
	if(conf_str)
		free(conf_str);
	if(conf2_str)
		free(conf2_str);
	return ret;
}


int is_mgmtd_snmp_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	return snmp_action_apply_to_snmpd_conf();
}

