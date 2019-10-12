/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "ubmc_mgmtd_func_def.h"
#include "silc_mgmtd_lib_err.h"
#include <signal.h>

#define LOCAL_IMAGE_PATH	"/var/images"
#define CDROM_OP_BIN_PATH	"/usr/sbin/ubmc_usb_cdrom_ctrl.sh"
#define CMD_BUFFER_SIZE		(512)

int is_mgmtd_bmc_action_power(silc_mgmtd_node* p_node)
{
	static char* op_arg_map[5][2] = {
			{"on",			"-o"},
			{"off",			"-s"},
			{"forceoff",	"-f"},
			{"reset",		"-r"},
			{"cycle",		"-c"},
	};
	silc_cstr op;
	silc_cstr arg = NULL;
	char cmd[64], out[64];
	int loop, len=64;

	if(strcmp(p_node->name, "control"))
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;

	op = p_node->tmp_value.val.string_val;
	SILC_LOG("BMC host power control: %s", op);
	for(loop=0; loop<5; loop++)
	{
		if(strcmp(op, op_arg_map[loop][0]) == 0)
		{
			arg = op_arg_map[loop][1];
			break;
		}
	}
	if(!arg)
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;

	sprintf(cmd, "ubmc_sys_ctrl.sh %s", arg);
	if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 10000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec '%s' error '%s'", cmd, silc_mgmtd_lib_err_str());
		return IS_MGMTD_ERR_BASE_EXEC_FAILED;
	}
	if(strncmp(out, "ok", 2) != 0)
	{
		SILC_ERR("Command '%s' ret '%s'", cmd, out);
		return IS_MGMTD_ERR_BASE_EXEC_FAILED;
	}

	return 0;
}

int is_mgmtd_bmc_action_bios(silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_image, *p_all;
	silc_cstr img;
	silc_bool all = silc_false;
	char cmd[64], out[64];
	int len=64;

	if(strcmp(p_node->name, "upgrade"))
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	p_image = silc_mgmtd_memdb_find_sub_node(p_node, "image");
	if(!p_image || p_image->tmp_value.type == SILC_MGMTD_VAR_NULL)
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	img = p_image->tmp_value.val.string_val;
	p_all = silc_mgmtd_memdb_find_sub_node(p_node, "all");
	if(p_all && p_all->tmp_value.type != SILC_MGMTD_VAR_NULL)
		all = p_all->tmp_value.val.bool_val;

	if(all)
		sprintf(cmd, "ubmc_sys_ctrl.sh -u %s &", img);
	else
		sprintf(cmd, "ubmc_sys_ctrl.sh -b %s &", img);
	if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 10000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec '%s' error '%s'", cmd, silc_mgmtd_lib_err_str());
		return IS_MGMTD_ERR_BASE_EXEC_FAILED;
	}
	SILC_LOG("Upgrade the host BIOS flash");

	return 0;
}

int is_mgmtd_bmc_action_sel(silc_mgmtd_node* p_node)
{
	char cmd[256], out[64];
	int len=64;
	int ret = 0;
	long int ubmc_ipmi_pid = 0;
	char *endptr;

	if(strcmp(p_node->name, "clear") == 0)
	{
		sprintf(cmd, "ps x| grep -w ubmc_ipmi|grep -v grep |awk '{print $1}'");
		if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 10000, silc_false) != 0)
		{
			SILC_ERR("Fail to exec '%s' error '%s'", cmd, silc_mgmtd_lib_err_str());
			return IS_MGMTD_ERR_BASE_EXEC_FAILED;
		}
        errno = 0;
		ubmc_ipmi_pid = strtol(out,&endptr,0);
        /* Check for various possible errors */
        if ((errno == ERANGE)|| (errno != 0 && ubmc_ipmi_pid == 0))
        {
        	SILC_ERR("Fail to exec '%s' error '%s'", cmd, strerror(errno));
        	return -1;
        }
        if (endptr == out)
        {
        	SILC_ERR("Fail to find ubmc_ipmi pid \n");
        	return -1;
        }
        ret = kill(ubmc_ipmi_pid,SIGUSR1);
        if(ret < 0)
        {
        	SILC_ERR("Fail to send signal to ubmc ipmi for clear, error: %s \n",strerror(errno));
        	return -1;
        }
		SILC_LOG("Clear all SEL log entries");
		return 0;
	}
	else
	{
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	return 0;
}

int is_mgmtd_remove_image(silc_mgmtd_node* p_node)
{
	char cmd[CMD_BUFFER_SIZE];
	char out[512];
	int len = 512;
	silc_cstr filename = p_node->tmp_value.val.string_val;

	snprintf(cmd,CMD_BUFFER_SIZE,"%s -r %s/%s",CDROM_OP_BIN_PATH, LOCAL_IMAGE_PATH, filename);
	if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 100000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec '%s' error '%s'", cmd, silc_mgmtd_lib_err_str());
		return IS_MGMTD_ERR_BASE_EXEC_FAILED;
	}
	if(strstr(out,"ISO file in use") != NULL)
	{
		return IS_MGMTD_ERR_BMC_CDROM_ISO_FILE_IN_USE;
	}
	else if(strstr(out,"ISO file not found") != NULL)
	{
		return IS_MGMTD_ERR_BMC_CDROM_ISO_FILE_NOT_FOUND;
	}
	SILC_LOG("Remove local image file %s", filename);
	return 0;
}

int is_mgmtd_bmc_action_usbcdrom(silc_mgmtd_node* p_node)
{
	char cmd[CMD_BUFFER_SIZE];
	char out[512];
	int len;

	//is_mgmtd_bmc_action_list_all_node(p_node->p_parent);

	memset(cmd,0,CMD_BUFFER_SIZE);
	if(strcmp(p_node->name, "attach-local") == 0)
	{
		silc_mgmtd_node *p_image;
		silc_cstr img = NULL;
		p_image = silc_mgmtd_memdb_find_sub_node(p_node, "image");
		if(!p_image || p_image->tmp_value.type == SILC_MGMTD_VAR_NULL)
			return IS_MGMTD_ERR_BASE_MISS_PARAM;
		img = p_image->tmp_value.val.string_val;

		snprintf(cmd,CMD_BUFFER_SIZE,"%s -a %s/%s",CDROM_OP_BIN_PATH, LOCAL_IMAGE_PATH, img);
		SILC_LOG("Attach the host USB CD-ROM with a local image: %s", img);
	}
	else if(strcmp(p_node->name, "attach-remote") == 0)
	{
		silc_mgmtd_node *p_addr, *p_path, *p_user, *p_passwd;
		silc_cstr addr = NULL, path = NULL, user = NULL, passwd = NULL;
		p_addr = silc_mgmtd_memdb_find_sub_node(p_node, "address");
		p_path = silc_mgmtd_memdb_find_sub_node(p_node, "path");
		p_user = silc_mgmtd_memdb_find_sub_node(p_node, "user");
		p_passwd = silc_mgmtd_memdb_find_sub_node(p_node, "password");
		if(!p_addr || p_addr->tmp_value.type == SILC_MGMTD_VAR_NULL ||
				!p_path || p_path->tmp_value.type == SILC_MGMTD_VAR_NULL ||
				!p_user || p_user->tmp_value.type == SILC_MGMTD_VAR_NULL ||
				!p_passwd || p_passwd->tmp_value.type == SILC_MGMTD_VAR_NULL)
			return IS_MGMTD_ERR_BASE_MISS_PARAM;
		addr = p_addr->tmp_value.val.string_val;
		path = p_path->tmp_value.val.string_val;
		user = p_user->tmp_value.val.string_val;
		passwd = p_passwd->tmp_value.val.string_val;

		snprintf(cmd,CMD_BUFFER_SIZE,"%s -A %s %s %s %s",CDROM_OP_BIN_PATH, addr ,path ,user ,passwd);
		SILC_LOG("Attach the host USB CD-ROM with a remote image: %s:%s", addr, path);
	}
	else if(strcmp(p_node->name, "detach") == 0)
	{
		snprintf(cmd,CMD_BUFFER_SIZE,"%s -d",CDROM_OP_BIN_PATH);
		SILC_LOG("Detach the host USB CD-ROM ");
	}
	else if(strcmp(p_node->name, "remove-local-isofile") == 0)
	{
		return is_mgmtd_remove_image(p_node);
	}

	if(strlen(cmd) > 0)
	{
		len = 512;
		if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 100000, silc_false) != 0)
		{
			SILC_ERR("Fail to exec '%s' error: %s", cmd, silc_mgmtd_lib_err_str());
			return IS_MGMTD_ERR_BASE_EXEC_FAILED;
		}

		//cannot find string ok
		if(strstr(out,"OK") == NULL)
		{
			SILC_ERR("Error: %s", out);
			if(strstr(out,"Permission denied") != NULL)
			{
				return IS_MGMTD_ERR_BMC_CDROM_USERNAME_OR_PASSWD_ERR;
			}
			else if(strstr(out,"Connection refused") != NULL
					|| strstr(out,"No route to host") != NULL
					|| strstr(out,"No such device") != NULL)
			{
				return IS_MGMTD_ERR_BMC_CDROM_REMOTE_HOST_UNAVAILABLE;
			}
			else if(strstr(out,"File not found") != NULL)
			{
				return IS_MGMTD_ERR_BMC_CDROM_ISO_FILE_NOT_FOUND;
			}
			else if(strstr(out,"CDROM attached") != NULL)
			{
				return IS_MGMTD_ERR_BMC_CDROM_ATTACHED;
			}
			else if(strstr(out,"CDROM not attached") != NULL)
			{
				return IS_MGMTD_ERR_BMC_CDROM_NOT_ATTACHED;
			}
			else
			{
				return IS_MGMTD_ERR_BASE_EXEC_FAILED;
			}
		}
	}
	return 0;
}

int is_mgmtd_bmc_action_phonehome(silc_mgmtd_node* p_node)
{
	char* cmd = "ubmc_phonehome.sh status";
	char out[256]={0};
	int len=256;

	if(strcmp(p_node->name, "enabled") != 0)
	{
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec '%s' error '%s'", cmd, silc_mgmtd_lib_err_str());
		return IS_MGMTD_ERR_BASE_EXEC_FAILED;
	}
	if(p_node->tmp_value.val.bool_val)
	{
		if(strstr(out, "Running"))
		{
			SILC_LOG("Phonehome service is already started");
			return 0;
		}
		SILC_LOG("Phonehome service is started");
		system("ubmc_phonehome.sh enable; ubmc_phonehome.sh start &");
	}
	else
	{
		if(strstr(out, "Disabled"))
		{
			SILC_LOG("Phonehome service is already stopped");
			return 0;
		}
		SILC_LOG("Phonehome service is stopped");
		system("ubmc_phonehome.sh disable");
	}
	return 0;
}

int is_mgmtd_bmc_action_images(silc_mgmtd_node* p_node)
{
	if(strcmp(p_node->name, "remove-local-image") == 0)
	{
		return is_mgmtd_remove_image(p_node);
	}
	return 0;
}

int is_mgmtd_bmc_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	if(type != SILC_MGMTD_IF_REQ_ACTION)
		return 0;

	if(strcmp(p_node->p_parent->name, "power") == 0)
	{
		return is_mgmtd_bmc_action_power(p_node);
	}
	else if(strcmp(p_node->p_parent->name, "bios") == 0)
	{
		return is_mgmtd_bmc_action_bios(p_node);
	}
	else if(strcmp(p_node->p_parent->name, "usb-cdrom") == 0)
	{
		return is_mgmtd_bmc_action_usbcdrom(p_node);
	}
	else if(strcmp(p_node->p_parent->name, "sel") == 0)
	{
		return is_mgmtd_bmc_action_sel(p_node);
	}
	else if(strcmp(p_node->p_parent->name, "phonehome") == 0)
	{
		return is_mgmtd_bmc_action_phonehome(p_node);
	}
	else if(strcmp(p_node->p_parent->name, "images") == 0)
	{
		return is_mgmtd_bmc_action_images(p_node);
	}

	return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
}

