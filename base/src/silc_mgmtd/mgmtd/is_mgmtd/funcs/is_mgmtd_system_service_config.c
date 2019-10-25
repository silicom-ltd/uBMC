/* this file is created by silc_mgmtd_inst_func_source_gen.py */

#include "silc_common.h"
#include "silc_common/silc_thread.h"
#include "is_mgmtd_func_def.h"
#include "silc_mgmtd_config.h"

int is_mgmtd_get_vrf_process_config(silc_cstr proc, silc_cstr* vrf_list, int len);
int is_mgmtd_run_vrf_process_cmd(silc_cstr proc, silc_cstr cmd);

#define IS_MGMTD_SYSTEM_SERVICE_PATH	"/config/system/service"

void silc_mgmtd_update_ts(char* module, void* conn_entry);

void is_mgmtd_system_service_update_ts(void* conn_entry)
{
	silc_mgmtd_update_ts("sys_service", conn_entry);
}

void is_trip_last_lf(char* str)
{
	int l = strlen(str);
	if (l > 0 && str[l-1] == '\n')
		str[l-1] = 0;
}

int is_file_exist(silc_cstr path)
{
	return (access(path, F_OK) == 0);
}

int is_write_file(silc_cstr path, silc_cstr buf, int len)
{
	FILE* fp;

	fp = fopen(path, "w");
	fwrite(buf, len, 1, fp);
	fflush(fp);
	fclose(fp);

	return 0;
}

int is_verify_ssl_cert(silc_cstr path)
{
	char cmd[256];
	char output[256];
	int len = 256;

	sprintf(cmd, "openssl x509 -in %s -noout", path);
	if (silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 1000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec cmd: %s", cmd);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	if (strstr(output, "error"))
	{
		SILC_ERR("Failed to verify SSL cert: %s", output);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}

	return 0;
}

int is_verify_ssl_key(silc_cstr path)
{
	char cmd[256];
	char output[256];
	int len = 256;

	sprintf(cmd, "openssl rsa -in %s -noout -check", path);
	if (silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 1000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec cmd: %s", cmd);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	if (strstr(output, "error"))
	{
		SILC_ERR("Failed to verify SSL key: %s", output);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}

	return 0;
}

int is_verify_ssl_cert_key(silc_cstr cert, silc_cstr key)
{
	char cmd[256];
	char output[256];
	int len = 256;

	sprintf(cmd, "(openssl x509 -noout -modulus -in %s ;openssl rsa -noout -modulus -in %s)|uniq|wc -l",
			cert, key);
	if (silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 1000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec cmd: %s", cmd);
		return IS_MGMTD_ERR_SYSTEM_CERT_KEY_MISMATCH;
	}
	if (strncmp(output, "1", 1) != 0)
	{
		SILC_ERR("Failed to verify SSL key and cert: %s", output);
		return IS_MGMTD_ERR_SYSTEM_CERT_KEY_MISMATCH;
	}

	return 0;
}

void is_mgmtd_get_mdf_node_val(const silc_cstr node_path, silc_cstr buff, int buff_len, silc_bool* modified)
{
	silc_mgmtd_node* p_node = silc_mgmtd_memdb_find_node(node_path);
	if (!p_node)
	{
		SILC_ERR("Can't fine node for %s!\n", node_path);
		return;
	}
	silc_mgmtd_var_to_str(&p_node->value, buff, buff_len);
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		*modified = silc_true;
		/* set it to null so it won't be triggered again */
		silc_mgmtd_var_clear(&p_node->tmp_value);
	}
}

#define IS_INITTAB_FILE		"/config/inittab"

int is_mgmtd_system_service_check_com()
{
	silc_mgmtd_node* p_node;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/com/speed");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		uint32_t speed_set[3] = {9600, 38400, 115200};
		uint32_t speed = p_node->tmp_value.val.uint32_val;
		int i;
		for (i=0; i<3; i++)
		{
			if (speed == speed_set[i])
				break;
		}
		if (i == 3)
		{
			SILC_ERR("Invalid COM speed %u", speed);
			return IS_MGMTD_ERR_SYSTEM_COM_SPEED_INVALID;
		}
	}
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/com/termtype");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		silc_cstr types[7] = {"xterm", "vt100", "vt102", "vt200", "vt220", "ansi", "linux"};
		int i;
		for(i=0; i<7; i++)
		{
			if(strcmp(types[i], p_node->tmp_value.val.string_val) == 0)
				return 0;
		}
		SILC_ERR("Invalid COM terminal type %s", p_node->tmp_value.val.string_val);
		return IS_MGMTD_ERR_SYSTEM_TERMINAL_TYPE_INVALID;
	}
	return 0;
}

int __is_mgmtd_system_service_config_com(void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_bool modified = silc_false;
	silc_cstr_array *p_arr = NULL;
	char speed[20];
	char termtype[20];
	char buff[256];
	char *line;
	int len = 200, loop;
	int ret = IS_MGMTD_ERR_SYSTEM_SET_COM_FAILED;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/com/speed", speed, 20, &modified);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/com/termtype", termtype, 20, &modified);
	if (!modified)
	{
		return 0;
	}
	USER_OP_LOG(conn_entry, "System COM is configured: speed %s termtype %s", speed, termtype);

	if (silc_mgmtd_if_exec_system_cmd("fw_printenv bootargs|sed 's/bootargs=//g'", buff, &len, 2000, silc_false) != 0)
		goto OUT;
	p_arr = silc_cstr_split(buff, " \n\r\t");
	if(!p_arr)
		goto OUT;
	strcpy(buff, "fw_setenv bootargs '");
	modified = silc_false;
	for(loop=0; loop<p_arr->length; loop++)
	{
		line = silc_cstr_array_get_quick(p_arr, loop);
		if(strstr(line, "console="))
		{
			if(!strstr(line, speed) || !strstr(line, termtype))
			{
				modified = silc_true;
				sprintf(buff+strlen(buff), "console=ttyS0,%sn8,%s ", speed, termtype);
			}
		}
		else
			sprintf(buff+strlen(buff), "%s ", line);
	}
	if(modified)
	{
		sprintf(buff+strlen(buff), "';fw_setenv baudrate %s", speed);
		if (silc_mgmtd_if_exec_system_cmd(buff, NULL, NULL, 2000, silc_false) != 0)
			goto OUT;
	}
	ret = 0;
OUT:
	if(p_arr)
		silc_cstr_array_destroy(p_arr);
	return ret;
#endif

    return 0;
}

int is_mgmtd_system_service_config_com(void* conn_entry)
{
	if(silc_mgmtd_memdb_get_product_info()->com_baudrate_support)
	{
		return __is_mgmtd_system_service_config_com(conn_entry);
	}
	return 0;
}
#define IS_SSH_CONFIG_DIR	"/etc/ssh"
#define IS_SSH_CONFIG_FILE	IS_SSH_CONFIG_DIR"/sshd_config"

#define IS_SSH_KEY_FILE_NUM	10

const char* ssh_node_keyfile[IS_SSH_KEY_FILE_NUM][2] =
{
	{"dsa-key", "ssh_host_dsa_key"},
	{"dsa-key-pub", "ssh_host_dsa_key.pub"},
	{"ecdsa-key", "ssh_host_ecdsa_key"},
	{"ecdsa-key-pub", "ssh_host_ecdsa_key.pub"},
	{"ed25519-key", "ssh_host_ed25519_key"},
	{"ed25519-key-pub", "ssh_host_ed25519_key.pub"},
	{"rsa-key", "ssh_host_rsa_key"},
	{"rsa-key-pub", "ssh_host_rsa_key.pub"},
	{"host-key", "ssh_host_key"},
	{"host-key-pub", "ssh_host_key.pub"},
};

int is_mgmtd_system_service_ssh_sync_key2db()
{
	silc_mgmtd_node* p_node;
	silc_mgmtd_var val;
	FILE* fp;
	struct stat st;
	int i, len, ret;
	char path[128];
	uint8_t* p_buf;

	if (!is_file_exist(IS_SSH_CONFIG_DIR"/ssh_host_rsa_key"))
	{
		SILC_LOG("SSH key files are absent, start sshd to generate them!\n");
		system("/etc/init.d/IS_S50sshd start");
	}

	for (i=0; i<IS_SSH_KEY_FILE_NUM; i++)
	{
		sprintf(path, "%s/%s", IS_SSH_CONFIG_DIR, ssh_node_keyfile[i][1]);
		if (!is_file_exist(path))
		{
			continue;
		}
		stat(path, &st);
		len = st.st_size;
		p_buf = (uint8_t*)malloc(len+1);
		if (!p_buf)
		{
			SILC_ERR("Malloc error!!!!\n");
			return IS_MGMTD_ERR_BASE_MALLOC_FAILED;
		}
		fp = fopen(path, "r");
		fread(p_buf, len, 1, fp);
		fclose(fp);

		sprintf(path, "%s/ssh/%s", IS_MGMTD_SYSTEM_SERVICE_PATH, ssh_node_keyfile[i][0]);
		p_node = silc_mgmtd_memdb_find_node(path);
		val.type = p_node->value.type;
		val.val.hex_val.data = p_buf;
		val.val.hex_val.len = len;
		silc_mgmtd_var_copy(&val, &p_node->value);
		memset(&val, 0x0, sizeof(val));
		free(p_buf);
	}

	ret = silc_mgmtd_cfg_save_config_to_file();
	if (ret)
		SILC_ERR("Save mgmtd config file error!\n");

	return ret;
}

int is_mgmtd_system_service_ssh_sync_db2key()
{
	static int synced = 0;
	silc_mgmtd_node* p_node;
	int i, len, ret;
	char path[128], path_bak[128];
	uint8_t* p_buf;

	if (synced)
	{
		return 0;
	}
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/ssh/rsa-key");
	if (p_node->value.val.hex_val.len == 0)
	{
		SILC_LOG("SSH key db nodes are null, sync from key files!\n");
		ret = is_mgmtd_system_service_ssh_sync_key2db();
		if (!ret)
			synced = 1;
		return ret;
	}

	for (i=0; i<IS_SSH_KEY_FILE_NUM; i++)
	{
		sprintf(path, "%s/ssh/%s", IS_MGMTD_SYSTEM_SERVICE_PATH, ssh_node_keyfile[i][0]);
		p_node = silc_mgmtd_memdb_find_node(path);
		p_buf = p_node->value.val.hex_val.data;
		len = p_node->value.val.hex_val.len;
		if (len > 0)
		{
			sprintf(path, "%s/%s", IS_SSH_CONFIG_DIR, ssh_node_keyfile[i][1]);
			sprintf(path_bak, "%s/%s.bak", IS_SSH_CONFIG_DIR, ssh_node_keyfile[i][1]);
			is_write_file(path_bak, (silc_cstr)p_buf, len);
			rename(path_bak, path);
			if (!strstr(ssh_node_keyfile[i][1], ".pub"))
				chmod(path, S_IRUSR|S_IWUSR);
		}
	}

	synced = 1;
	return 0;
}

#if 0
#define IS_VRF_NUM_MAX	8

int is_mgmtd_check_vrf_address(silc_cstr vrf, silc_cstr_array* addr_list)
{
	int len = 2048;
	static char out[2048];
	char cmd[128], addr_tag[128];
	int ret, i;
	silc_cstr addr;

	if(vrf && vrf[0])
		sprintf(cmd, "ip route show vrf %s", vrf);
	else
		strcpy(cmd, "ip route show");
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 3000, silc_false);
	if(ret)
	{
		SILC_ERR("Fail to exec cmd: '%s', ret: %d", cmd, ret);
		return ret;
	}
	for(i=0; i<addr_list->length; i++)
	{
		addr = silc_cstr_array_get_quick(addr_list, i);
		sprintf(addr_tag, "src %s", addr);
		if(strstr(out, addr_tag) == NULL)
		{
			SILC_LOG("VRF [%s] address %s is not ready yet", vrf, addr);
			return -1;
		}
	}

	return 0;
}

int is_mgmtd_start_vrf_ssh(silc_cstr vrf, silc_cstr_array* addr_list, int ssh_port)
{
	char conf[64], cmd[256];
	FILE* fp_orig;
	FILE* fp_conf;
	char buff[512];
	int i;

	system("/usr/bin/ssh-keygen -A");
	if (vrf && vrf[0])
	{
		sprintf(conf, "/etc/ssh/sshd_vrf_%s_config", vrf);
		sprintf(cmd, "ip vrf exec %s /usr/sbin/sshd -f %s", vrf, conf);
	}
	else
	{
		sprintf(conf, "/etc/ssh/sshd_new_config");
		sprintf(cmd, "/usr/sbin/sshd -f %s", conf);
	}

	fp_orig = fopen(IS_SSH_CONFIG_FILE, "r");
	fp_conf = fopen(conf, "w");
	while (fgets(buff, 512, fp_orig))
	{
		if (strncmp(buff, "Port", strlen("Port")) == 0 ||
				strncmp(buff, "ListenAddress", strlen("ListenAddress")) == 0)
		{
			continue;
		}
		fprintf(fp_conf, "%s", buff);
	}
	fprintf(fp_conf, "Port %d\n", ssh_port);
	for(i=0; addr_list && i<addr_list->length; i++)
	{
		fprintf(fp_conf, "ListenAddress %s\n", silc_cstr_array_get_quick(addr_list, i));
	}

	fflush(fp_conf);
	fclose(fp_conf);
	fclose(fp_orig);

	system(cmd);

	return 0;
}

void* is_mgmtd_start_ssh_thread(void* thread_arg)
{
	silc_cstr vrf_list[IS_VRF_NUM_MAX]={NULL}, bind_addr_list[IS_VRF_NUM_MAX]={NULL};
	silc_cstr_array* addr_list[IS_VRF_NUM_MAX] = {NULL};
	int ready[IS_VRF_NUM_MAX] = {0};
	int i, vrf_num, not_ready;
	int ssh_port = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/ssh/port")->value.val.uint32_val;

	system("killall -q sshd");

	vrf_num = is_mgmtd_get_vrf_process_config("ssh", vrf_list, bind_addr_list, IS_VRF_NUM_MAX);
	if(0 == vrf_num)
	{
		is_mgmtd_start_vrf_ssh(NULL, NULL, ssh_port);
		return NULL;
	}
	for(i=0; i<vrf_num; i++)
	{
		addr_list[i] = silc_cstr_split(bind_addr_list[i], ",");
		if(!addr_list[i])
		{
			SILC_ERR("silc_cstr_split error");
			return NULL;
		}
	}

	while (1)
	{
		not_ready = 0;
		for(i=0; i<vrf_num; i++)
		{
			if(ready[i])
				continue;
			if(is_mgmtd_check_vrf_address(vrf_list[i], addr_list[i]))
			{
				not_ready = 1;
				continue;
			}
			if(is_mgmtd_start_vrf_ssh(vrf_list[i], addr_list[i], ssh_port))
			{
				SILC_ERR("Fail to start sshd");
				not_ready = 1;
			}
			else
				ready[i] = 1;
		}
		if(not_ready)
			sleep(3);
		else
			break;
	}
	SILC_LOG("SSH is ready");

	for(i=0; i<vrf_num; i++)
	{
		silc_cstr_array_destroy(addr_list[i]);
		addr_list[i] = NULL;
	}
	return NULL;
}

int is_mgmtd_start_ssh()
{
	// start a thread to wait listen address to be ready
	return silc_thread_create_detached(is_mgmtd_start_ssh_thread, NULL);
}
#endif
int is_mgmtd_start_ssh(int ssh_port)
{
	FILE* fp;
	FILE* fp_bak;
	char buff[512];

	system("killall -q sshd; /usr/bin/ssh-keygen -A");

	fp = fopen(IS_SSH_CONFIG_FILE, "r");
	fp_bak = fopen(IS_SSH_CONFIG_FILE".bak", "w");
	while (fgets(buff, 512, fp))
	{
		if (strncmp(buff, "Port", 4) == 0)
		{
			continue;
		}
		fprintf(fp_bak, "%s", buff);
	}
	fprintf(fp_bak, "Port %d\n", ssh_port);
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(IS_SSH_CONFIG_FILE".bak", IS_SSH_CONFIG_FILE);

	is_mgmtd_run_vrf_process_cmd("ssh", "/usr/sbin/sshd");

	return 0;
}

int is_mgmtd_system_service_config_ssh(void* conn_entry)
{
	silc_bool modified = silc_false;
	char buff[512];
	silc_bool ssh_en;
	int ssh_port;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/ssh/enabled", buff, 512, &modified);
	ssh_en = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/ssh/port", buff, 512, &modified);
	ssh_port = atoi(buff);
	if (!modified)
	{
		return 0;
	}
	USER_OP_LOG(conn_entry, "System SSH is configured: enable %d, port %d", ssh_en, ssh_port);
	if (is_file_exist("/etc/init.d/S50sshd"))
	{
		// rename it so rcS won't start it later
		system("mv /etc/init.d/S50sshd /etc/init.d/IS_S50sshd");
	}
	/* SSH is not allowed to shutup
	if (!ssh_en)
	{
		system("/etc/init.d/IS_S50sshd stop");
		return 0;
	}
	*/

	is_mgmtd_system_service_ssh_sync_db2key();

	is_mgmtd_start_ssh(ssh_port);

	return 0;
}

#define IS_NETCONF_PORT			830
#define IS_LUCI_UHTTPD_PORT		58080
#define IS_LUCI_CONFIG_FILE		"/opt/silc-web/host/etc/config/luci"
#define IS_NGINX_CONFIG_FILE	"/etc/nginx/nginx.conf"

int is_mgmtd_get_reserved_iptables_rule_num(silc_cstr version)
{
	silc_cstr bin = "iptables";
	char cmd[256];
	char output[256];
	int len = 256;

	if(strcmp(version, "ipv6") == 0)
		bin = "ip6tables";
	sprintf(cmd, "%s -L | grep dpt:%d | wc -l", bin, IS_LUCI_UHTTPD_PORT);
	if (silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 1000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec cmd: %s", cmd);
		return 0;
	}
	return atoi(output);
}

int is_mgmtd_start_luci()
{
	char* backend = "0.0.0.0";
	silc_cstr vrf_list[4] = {NULL};
	int num, i;
	char cmd[256];

	system("killall -q uhttpd");

	num = is_mgmtd_get_vrf_process_config("web", vrf_list, 4);
	if(num > 0)
	{
		for(i=0; i<num; i++)
		{
			// drop all incoming packets not from local address
			sprintf(cmd, "iptables -I INPUT -p tcp --dport %d -i %s -m mac ! --mac-source $(cat /sys/class/net/%s/address) -j DROP",
					IS_LUCI_UHTTPD_PORT, vrf_list[i], vrf_list[i]);
			system(cmd);
		}
	}
	else
	{
		sprintf(cmd, "iptables -I INPUT -p tcp --dport %d ! -s 127.0.0.1 -j DROP", IS_LUCI_UHTTPD_PORT);
		system(cmd);
	}

	sprintf(cmd, "/opt/silc-web/silc-web-start.sh %s:%d &", backend, IS_LUCI_UHTTPD_PORT);
	is_mgmtd_run_vrf_process_cmd("web", cmd);

	return 0;
}

int is_mgmtd_system_service_config_luci(void* conn_entry)
{
	silc_bool modified = silc_false;
	char buff[256];
	char path[256];
	int timeout;
	FILE* fp;
	FILE* fp_bak;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/web/session-path", buff, 256, &modified);
	strcpy(path, buff);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/web/session-timeout", buff, 256, &modified);
	timeout = atoi(buff);
	if (!modified)
	{
		return 0;
	}
	USER_OP_LOG(conn_entry, "System Web is configured: session-timeout %d", timeout);

	fp = fopen(IS_LUCI_CONFIG_FILE, "r");
	fp_bak = fopen(IS_LUCI_CONFIG_FILE".bak", "w");
	while (fgets(buff, 512, fp))
	{
		if (strstr(buff, "sessiontime"))
		{
			fprintf(fp_bak, "	option sessiontime %d\n", timeout);
		}
		else if (strstr(buff, "sessionpath"))
		{
			fprintf(fp_bak, "	option sessionpath %s\n", path);
		}
		else
			fprintf(fp_bak, "%s", buff);
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(IS_LUCI_CONFIG_FILE".bak", IS_LUCI_CONFIG_FILE);

	is_mgmtd_start_luci();
	return 0;
}
#if 0
int is_mgmtd_get_vrf_local_addr(silc_cstr vrf, silc_cstr addr)
{
	int len = 256;
	char cmd[256], out[256];
	int ret;

	sprintf(cmd, "ip route show vrf %s|head -n 1|awk '{print $9}'", vrf);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 3000, silc_false);
	if(ret)
	{
		SILC_ERR("Fail to exec cmd: '%s', ret: %d", cmd, ret);
		return ret;
	}
	is_trip_last_lf(out);
	if(!out[0])
	{
		SILC_ERR("Fail to get VRF %s local address", vrf);
		return -1;
	}
	strcpy(addr, out);
	return 0;
}

int is_mgmtd_start_vrf_luci(silc_cstr vrf, silc_cstr addr)
{
	char cmd[256];

	if(!vrf || !vrf[0])
	{
		system("/opt/silc-web/silc-web-start.sh &");
		return 0;
	}

	SILC_LOG("Start luci with VRF=%s", vrf);
	// only accept local traffic
	sprintf(cmd, "iptables -I INPUT -p tcp -d %s --dport %d ! -s %s -j DROP", addr, IS_LUCI_UHTTPD_PORT, addr);
	system(cmd);
	sprintf(cmd, "ip vrf exec %s /opt/silc-web/silc-web-start.sh %s:%d &", vrf, addr, IS_LUCI_UHTTPD_PORT);
	system(cmd);
	return 0;
}

void is_mgmtd_get_nginx_conf_addr(silc_cstr addr, silc_cstr conf_addr)
{
	if(strstr(addr, ":"))
		sprintf(conf_addr, "[%s]", addr);
	else
		sprintf(conf_addr, "%s", addr);
}

int is_mgmtd_start_vrf_nginx(silc_cstr vrf, silc_cstr_array* addr_list, silc_cstr luci_addr,
		silc_bool http_en, int http_port, silc_bool https_en, int https_port)
{
	char conf[64], cmd[256], backend[64], conf_addr[64];
	FILE* fp;
	time_t t;
	int i;

	system("mkdir -p /var/log/nginx /var/tmp/nginx");
	if (vrf && vrf[0])
	{
		sprintf(conf, "/etc/nginx/nginx-vrf-%s.conf", vrf);
		sprintf(cmd, "ip vrf exec %s /usr/sbin/nginx -c %s", vrf, conf);
		is_mgmtd_get_nginx_conf_addr(luci_addr, backend);
	}
	else
	{
		strcpy(conf, "/etc/nginx/nginx.conf");
		strcpy(cmd, "/usr/sbin/nginx");
		strcpy(backend, "127.0.0.1");
	}

	fp = fopen(conf, "w");
	time(&t);
	fprintf(fp, "##\n"
				"## This file was AUTOMATICALLY GENERATED. DO NOT MODIFY IT.\n"
				"## All changes will be lost.\n"
				"##\n"
				"## Generated by %s at %s"
				"##\n", __func__, ctime(&t));

	fprintf(fp, "worker_processes  1;\n"
				"events {\n"
				"    worker_connections  1024;\n"
				"}\n"
				"http {\n"
				"    access_log     off;\n"
				"    error_log      /tmp/nginx_error.log;\n"
				"    include        mime.types;\n"
				"    default_type   application/octet-stream;\n"
				"    sendfile       off;\n"
				"    keepalive_timeout           600;\n"
				"    proxy_connect_timeout       600;\n"
				"    proxy_send_timeout          600;\n"
				"    proxy_read_timeout          600;\n"
				"    send_timeout                600;\n");

	if (http_en)
	{
		fprintf(fp, "    server {\n");
		if(!addr_list)
		{
			fprintf(fp,
				    "        listen       %d;\n"
			    	"        listen  [::]:%d;\n"
					, http_port, http_port);
		}
		else
		{
			for(i=0; i<addr_list->length; i++)
			{
				is_mgmtd_get_nginx_conf_addr(silc_cstr_array_get_quick(addr_list, i), conf_addr);
				fprintf(fp,
					"        listen  %s:%d;\n"
						, conf_addr, http_port);
			}
		}
		fprintf(fp,
			    	"        server_name  localhost;\n"
#ifdef IS_DIRECT_HTTP_ENABLE
				    "        client_max_body_size 100m;\n"
					"        add_header X-Frame-Options \"SAMEORIGIN\";\n"
				    "        location / {\n"
				    "            proxy_pass http://127.0.0.1:8080;\n"
				    "            proxy_redirect default;\n"
				    "        }\n"
				    "        error_page   500 502 503 504  /50x.html;\n"
				    "        location = /50x.html {\n"
				    "            root   html;\n"
				    "        }\n"
#else
					"        return 301 https://$host$request_uri;\n"
#endif
				    "    }\n");
	}

	if (https_en)
	{
		fprintf(fp, "    # HTTPS server\n"
					"    #\n"
					"    server {\n");
		if(!addr_list)
		{
			fprintf(fp,
					"        listen       %d ssl;\n"
					"        listen  [::]:%d ssl;\n"
				, https_port, https_port);
#ifndef IS_DIRECT_HTTP_ENABLE
			if (http_en && https_port != 443)
			{
				// http redirect to https on standard 443 port
				fprintf(fp,
					"        listen       443 ssl;\n"
					"        listen  [::]:443 ssl;\n");
			}
#endif
		}
		else
		{
			for(i=0; i<addr_list->length; i++)
			{
				is_mgmtd_get_nginx_conf_addr(silc_cstr_array_get_quick(addr_list, i), conf_addr);
				fprintf(fp,
					"        listen  %s:%d ssl;\n"
						, conf_addr, https_port);
#ifndef IS_DIRECT_HTTP_ENABLE
				if (http_en && https_port != 443)
				{
					// http redirect to https on standard 443 port
					fprintf(fp,
					"        listen  %s:443 ssl;\n"
							, conf_addr);
				}
#endif
			}
		}
		fprintf(fp,	"        server_name  localhost;\n"
				    "        client_max_body_size 1000m;\n"
					"        ssl_certificate      nginx_ssl.crt;\n"
					"        ssl_certificate_key  nginx_ssl.key;\n"
					"        ssl_session_cache    shared:SSL:1m;\n"
					"        ssl_session_timeout  10m;\n"
					"        ssl_ciphers  HIGH:!aNULL:!MD5;\n"
					"        ssl_prefer_server_ciphers  on;\n"
					"        ssl_protocols TLSv1.2 TLSv1.1;\n"
					"        add_header X-Frame-Options \"SAMEORIGIN\";\n"
					"        location / {\n"
					"            proxy_set_header Host $host;\n"
					"            proxy_set_header X-Real-IP $remote_addr;\n"
					"            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
					"            proxy_pass http://%s:%d;\n"
					"            proxy_redirect default;\n"
					"       }\n"
					"    }\n", backend, IS_LUCI_UHTTPD_PORT);
	}
	fprintf(fp, "}\n");

	fflush(fp);
	fclose(fp);

	system(cmd);
	return 0;
}

void* is_mgmtd_start_web_thread(void* thread_arg)
{
	silc_cstr vrf_list[IS_VRF_NUM_MAX] = {NULL}, bind_addr_list[IS_VRF_NUM_MAX] = {NULL};
	silc_cstr_array* addr_list[IS_VRF_NUM_MAX] = {NULL};
	int ready[IS_VRF_NUM_MAX] = {0};
	int i, vrf_num, not_ready;
	silc_bool http_en = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/http/enabled")->value.val.bool_val;
	int http_port = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/http/port")->value.val.uint32_val;
	silc_bool https_en = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/enabled")->value.val.bool_val;
	int https_port = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/port")->value.val.uint32_val;
	char local_addr[64];

	system("killall -q nginx uhttpd");
	if (!http_en && !https_en)
	{
		return 0;
	}

	vrf_num = is_mgmtd_get_vrf_process_config("web", vrf_list, bind_addr_list, IS_VRF_NUM_MAX);
	if(0 == vrf_num)
	{
		is_mgmtd_start_vrf_luci(NULL, NULL);
		is_mgmtd_start_vrf_nginx(NULL, NULL, NULL, http_en, http_port, https_en, https_port);
		return NULL;
	}
	for(i=0; i<vrf_num; i++)
	{
		addr_list[i] = silc_cstr_split(bind_addr_list[i], ",");
		if(!addr_list[i])
		{
			SILC_ERR("silc_cstr_split error");
			return NULL;
		}
	}

	while (1)
	{
		not_ready = 0;
		for(i=0; i<vrf_num; i++)
		{
			if(ready[i])
				continue;
			if(is_mgmtd_check_vrf_address(vrf_list[i], addr_list[i]))
			{
				not_ready = 1;
				continue;
			}
			if(vrf_list[i] && vrf_list[i][0] &&
					is_mgmtd_get_vrf_local_addr(vrf_list[i], local_addr))
			{
				not_ready = 1;
				continue;
			}
			if(is_mgmtd_start_vrf_luci(vrf_list[i], local_addr) ||
					is_mgmtd_start_vrf_nginx(vrf_list[i], addr_list[i], local_addr,
							http_en, http_port, https_en, https_port))
			{
				SILC_ERR("Fail to start luci/nginx");
				not_ready = 1;
			}
			else
				ready[i] = 1;
		}
		if(not_ready)
			sleep(3);
		else
			break;
	}
	SILC_LOG("Web is ready");

	for(i=0; i<vrf_num; i++)
	{
		silc_cstr_array_destroy(addr_list[i]);
		addr_list[i] = NULL;
	}
	return NULL;
}

int is_mgmtd_start_web()
{
	// start a thread to wait listen address to be ready
	return silc_thread_create_detached(is_mgmtd_start_web_thread, NULL);
}

silc_cstr_array* is_mgmtd_get_cmd_output_array(silc_cstr cmd)
{
	static char out[2048];
	int len = 2048, ret;
	silc_cstr_array* p_list = NULL;

	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 3000, silc_false);
	if(ret)
	{
		SILC_ERR("Fail to exec cmd: '%s', ret: %d", cmd, ret);
		return NULL;
	}
	if(!out[0])
		return NULL;
	p_list = silc_cstr_split(out, "\n");
	if(!p_list)
	{
		SILC_ERR("silc_cstr_split error");
		return NULL;
	}
	return p_list;
}
int is_mgmtd_filter_vrf_address(silc_cstr vrf)
{
	char cmd[256];
	silc_cstr_array* p_addr_list = NULL;
	silc_cstr addr;
	int i;

	sprintf(cmd, "ip route show vrf %s|grep -w src|awk '{print $9}'", vrf);
	p_addr_list = is_mgmtd_get_cmd_output_array(cmd);
	if(!p_addr_list)
	{
		SILC_LOG("VRF %s doesn't have valid address", vrf);
		return 0;
	}
	for(i=0; i<p_addr_list->length; i++)
	{
		addr = silc_cstr_array_get_quick(p_addr_list, i);
		SILC_LOG("Filter vrf %s addr %s", vrf, addr);
	}
	silc_cstr_array_destroy(p_addr_list);

	return 0;
}

void* is_mgmtd_vrf_scan_thread(void* thread_arg)
{
	silc_cstr_array* p_vrf_list = NULL;
	silc_cstr vrf;
	int i;

	p_vrf_list = is_mgmtd_get_cmd_output_array("ip vrf show|grep -v Name|grep -v '\\-----------------------'|awk '{print $1}'");
	if(!p_vrf_list)
	{
		SILC_LOG("No VRF is configured");
		return NULL;
	}

	while (1)
	{
		for(i=0; i<p_vrf_list->length; i++)
		{
			vrf = silc_cstr_array_get_quick(p_vrf_list, i);
			SILC_LOG("Filter vrf %s addr", vrf);
			is_mgmtd_filter_vrf_address(vrf);
		}
		sleep(3);
	}
	silc_cstr_array_destroy(p_vrf_list);

	return NULL;
}

int is_mgmtd_start_vrf_scan_thread()
{
	static int started = 0;
	// start a thread to scan vrf source address and set iptables rule to protect IS_LUCI_UHTTPD_PORT
	if(!started)
	{
		started = 1;
		return silc_thread_create_detached(is_mgmtd_vrf_scan_thread, NULL);
	}
	return 0;
}
#endif
int is_mgmtd_start_nginx(silc_bool http_en, int http_port, silc_bool https_en, int https_port)
{
	char* conf = "/etc/nginx/nginx.conf";
	char* backend = "0.0.0.0";
	FILE* fp;
	time_t t;

	system("killall -q nginx");
	system("mkdir -p /var/log/nginx /var/tmp/nginx");

	fp = fopen(conf, "w");

	time(&t);
	fprintf(fp, "##\n"
				"## This file was AUTOMATICALLY GENERATED. DO NOT MODIFY IT.\n"
				"## All changes will be lost.\n"
				"##\n"
				"## Generated by %s at %s"
				"##\n", __func__, ctime(&t));

	fprintf(fp, "worker_processes  1;\n"
				"events {\n"
				"    worker_connections  1024;\n"
				"}\n"
				"http {\n"
				"    access_log     off;\n"
				"    error_log      /tmp/nginx_error.log;\n"
				"    include        mime.types;\n"
				"    default_type   application/octet-stream;\n"
				"    sendfile       off;\n"
				"    keepalive_timeout           600;\n"
				"    proxy_connect_timeout       600;\n"
				"    proxy_send_timeout          600;\n"
				"    proxy_read_timeout          600;\n"
				"    send_timeout                600;\n");

	if (http_en)
	{
		fprintf(fp,
				"    server {\n");
		fprintf(fp,
				"        listen       %d;\n"
				"        listen  [::]:%d;\n"
				, http_port, http_port);
		fprintf(fp,
				"        server_name  localhost;\n"
#ifdef IS_DIRECT_HTTP_ENABLE
				"        client_max_body_size 100m;\n"
				"        add_header X-Frame-Options \"SAMEORIGIN\";\n"
				"        location / {\n"
				"            proxy_pass http://127.0.0.1:8080;\n"
				"            proxy_redirect default;\n"
				"        }\n"
				"        error_page   500 502 503 504  /50x.html;\n"
				"        location = /50x.html {\n"
				"            root   html;\n"
				"        }\n"
#else
				"        return 301 https://$host$request_uri;\n"
#endif
				"    }\n");
	}

	if (https_en)
	{
		fprintf(fp,
				"    # HTTPS server\n"
				"    #\n"
				"    server {\n");
		fprintf(fp,
				"        listen       %d ssl;\n"
				"        listen  [::]:%d ssl;\n"
			, https_port, https_port);
#ifndef IS_DIRECT_HTTP_ENABLE
		if (http_en && https_port != 443)
		{
			// http redirect to https on standard 443 port
			fprintf(fp,
				"        listen       443 ssl;\n"
				"        listen  [::]:443 ssl;\n");
		}
#endif
		fprintf(fp,
				"        server_name  localhost;\n"
				"        client_max_body_size 1000m;\n"
				"        ssl_certificate      nginx_ssl.crt;\n"
				"        ssl_certificate_key  nginx_ssl.key;\n"
				"        ssl_session_cache    shared:SSL:1m;\n"
				"        ssl_session_timeout  10m;\n"
				"        ssl_ciphers  HIGH:!aNULL:!MD5;\n"
				"        ssl_prefer_server_ciphers  on;\n"
				"        ssl_protocols TLSv1.2 TLSv1.1;\n"
				"        add_header X-Frame-Options \"SAMEORIGIN\";\n"
				"        location / {\n"
				"            proxy_set_header Host $host;\n"
				"            proxy_set_header X-Real-IP $remote_addr;\n"
				"            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
				"            proxy_pass http://%s:%d;\n"
				"            proxy_redirect default;\n"
				"       }\n"
				"    }\n", backend, IS_LUCI_UHTTPD_PORT);
	}
	fprintf(fp, "}\n");

	fflush(fp);
	fclose(fp);

	is_mgmtd_run_vrf_process_cmd("web", "/usr/sbin/nginx");

	return 0;
}

#define IS_NGINX_CERT_FILE		"/etc/nginx/nginx_ssl.crt"
#define IS_NGINX_CERT_FILE_TMP	IS_NGINX_CERT_FILE".tmp"
#define IS_NGINX_KEY_FILE		"/etc/nginx/nginx_ssl.key"
#define IS_NGINX_KEY_FILE_TMP	IS_NGINX_KEY_FILE".tmp"
int is_mgmtd_system_service_config_nginx(void* conn_entry)
{
	silc_mgmtd_node* p_node;
	silc_bool modified = silc_false, ssl_modified = silc_false;
	char buff[64];
	silc_bool http_en, https_en;
	int http_port, https_port;
	silc_cstr cert, key;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/http/enabled", buff, 64, &modified);
	http_en = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/http/port", buff, 64, &modified);
	http_port = atoi(buff);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/enabled", buff, 64, &modified);
	https_en = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/port", buff, 64, &modified);
	https_port = atoi(buff);
	if (modified)
	{
		USER_OP_LOG(conn_entry, "System HTTP/HTTPS service is configured: http-enable %d, http-port %d, https-enable %d, https-port %d",
				http_en, http_port, https_en, https_port);
	}

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/certificate");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		ssl_modified = silc_true;
		/* set it to null so it won't be triggered again */
		silc_mgmtd_var_clear(&p_node->tmp_value);
		USER_OP_LOG(conn_entry, "System HTTPS cert is modified");
	}
	cert = p_node->value.val.string_val;
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/private-key");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		ssl_modified = silc_true;
		/* set it to null so it won't be triggered again */
		silc_mgmtd_var_clear(&p_node->tmp_value);
		USER_OP_LOG(conn_entry, "System HTTPS key is modified");
	}
	key = p_node->value.val.string_val;

	if (!modified && !ssl_modified)
	{
		return 0;
	}

	SILC_TRACE("Config http: %d-%d, https: %d-%d!\n", http_en, http_port, https_en, https_port);
	if (is_file_exist("/etc/init.d/S50nginx"))
	{
		// rename it so rcS won't start it later
		system("mv /etc/init.d/S50nginx /etc/init.d/IS_S50nginx");
	}
	if (!http_en && !https_en)
	{
		//system("/etc/init.d/IS_S50nginx stop");
		system("killall -q nginx uhttpd");
		return 0;
	}

	if (ssl_modified)
	{
		int has_cert, has_key;
		has_cert = is_file_exist(IS_NGINX_CERT_FILE_TMP);
		has_key = is_file_exist(IS_NGINX_KEY_FILE_TMP);
		if (!has_cert && !has_key)
		{
			// it is initialized from db file
			if (strlen(cert) > 0)
			{
				is_write_file(IS_NGINX_CERT_FILE_TMP, cert, strlen(cert));
				has_cert = 1;
			}
			if (strlen(key) > 0)
			{
				is_write_file(IS_NGINX_KEY_FILE_TMP, key, strlen(key));
				has_key = 1;
			}
		}
		ssl_modified = silc_false;
		if (has_cert && has_key)
		{
			if (0 == is_verify_ssl_cert_key(IS_NGINX_CERT_FILE_TMP, IS_NGINX_KEY_FILE_TMP))
			{
				SILC_LOG("SSL cert and key are matched, update them.\n");
				rename(IS_NGINX_CERT_FILE_TMP, IS_NGINX_CERT_FILE);
				rename(IS_NGINX_KEY_FILE_TMP, IS_NGINX_KEY_FILE);
				ssl_modified = silc_true;
			}
			else
				SILC_LOG("SSL cert and key are mismatched, ignore them.\n");
		}
		else if (has_cert || has_key)
			SILC_LOG("SSL cert and key are not ready yet, use original cert and key.\n");

		if (!ssl_modified && !modified)
			return 0;
	}

	is_mgmtd_start_nginx(http_en, http_port, https_en, https_port);
	return 0;
}

int is_mgmtd_system_service_config_telnet(void* conn_entry)
{
	return 0;
}

int is_mgmtd_check_port(uint32_t port)
{
	if (port > 65535)
	{
		SILC_ERR("Invalid port: %u", port);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	else if (port == IS_LUCI_UHTTPD_PORT)
	{
		SILC_ERR("Port conflict with luci port: %u", port);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	else if (port == IS_NETCONF_PORT)
	{
		SILC_ERR("Port conflict with netconf port: %u", port);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	return 0;
}

int is_mgmtd_check_port_node(const silc_cstr path)
{
	silc_mgmtd_node* p_node = silc_mgmtd_memdb_find_node(path);
	if (p_node && p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		return is_mgmtd_check_port(p_node->tmp_value.val.uint32_val);
	}
	return 0;
}

int is_mgmtd_check_session_timeout()
{
	silc_mgmtd_node* p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/web/session-timeout");
	if (p_node && p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		uint32_t timeout = p_node->tmp_value.val.uint32_val;
		if (timeout < 60)
		{
			SILC_ERR("Invalid session timeout: %u", timeout);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	return 0;
}

int is_mgmtd_check_ssl()
{
	silc_mgmtd_node* p_node;
	silc_cstr cert=NULL, key=NULL;
	int has_cert, has_key, ret;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/certificate");
	if (p_node && p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		cert = p_node->tmp_value.val.string_val;
		is_write_file(IS_NGINX_CERT_FILE_TMP, cert, strlen(cert));
		ret = is_verify_ssl_cert(IS_NGINX_CERT_FILE_TMP);
		if (ret)
		{
			unlink(IS_NGINX_CERT_FILE_TMP);
			return ret;
		}
	}

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/private-key");
	if (p_node && p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		key = p_node->tmp_value.val.string_val;
		is_write_file(IS_NGINX_KEY_FILE_TMP, key, strlen(key));
		ret = is_verify_ssl_key(IS_NGINX_KEY_FILE_TMP);
		if (ret)
		{
			unlink(IS_NGINX_KEY_FILE_TMP);
			return ret;
		}
	}

	has_cert = is_file_exist(IS_NGINX_CERT_FILE_TMP);
	has_key = is_file_exist(IS_NGINX_KEY_FILE_TMP);
	if (has_cert && has_key)
	{
		ret = is_verify_ssl_cert_key(IS_NGINX_CERT_FILE_TMP, IS_NGINX_KEY_FILE_TMP);
		if (ret)
		{
			if (cert)
				unlink(IS_NGINX_CERT_FILE_TMP);
			if (key)
				unlink(IS_NGINX_KEY_FILE_TMP);
			return ret;
		}
	}

	return 0;
}

int is_mgmtd_system_service_check_req(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	int ret;
	ret = is_mgmtd_system_service_check_com();
	if (ret)
		return ret;
	ret = is_mgmtd_check_port_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/ssh/port");
	if (ret)
		return ret;
	ret = is_mgmtd_check_port_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/http/port");
	if (ret)
		return ret;
	ret = is_mgmtd_check_port_node(IS_MGMTD_SYSTEM_SERVICE_PATH"/https/port");
	if (ret)
		return ret;
	ret = is_mgmtd_check_session_timeout();
	if (ret)
		return ret;
	ret = is_mgmtd_check_ssl();
	if (ret)
		return ret;
	return 0;
}

int is_mgmtd_system_service_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	int ret;

	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
			return is_mgmtd_system_service_check_req(type, p_db_node, conn_entry);
		case SILC_MGMTD_IF_REQ_MODIFY:
			is_mgmtd_system_service_update_ts(conn_entry);
			break;
		default:
			return 0;
	}

	ret = is_mgmtd_system_service_config_com(conn_entry);
	if (ret)
		return ret;

	ret = is_mgmtd_system_service_config_ssh(conn_entry);
	if (ret)
		return ret;

	ret = is_mgmtd_system_service_config_luci(conn_entry);
	if (ret)
		return ret;

	ret = is_mgmtd_system_service_config_nginx(conn_entry);
	if (ret)
		return ret;

	ret = is_mgmtd_system_service_config_telnet(conn_entry);
	if (ret)
		return ret;
#endif
    return 0;
}

