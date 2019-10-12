
#include "silc_common.h"
#include "silc_mgmtd_error.h"
#include "silc_mgmtd_memdb.h"
#include "silc_mgmtd_operation.h"


#define SILC_MGMTD_CONFIG_FILE_NAME_MAX_LEN	256
#define SILC_MGMTD_CONFIG_READ_LINE_MAX_LEN	(1024*16)

typedef struct silc_mgmtd_cfg_ctl_s
{
	char filename[SILC_MGMTD_CONFIG_FILE_NAME_MAX_LEN];
	char running_config_filename[SILC_MGMTD_CONFIG_FILE_NAME_MAX_LEN];
	char read_buff[SILC_MGMTD_CONFIG_READ_LINE_MAX_LEN];
	silc_mutex *write_lock;
}silc_mgmtd_cfg_ctl;

static silc_mgmtd_cfg_ctl s_silc_mgmtd_config_ctl;


/*
 * --------------------------------- FILE CHECK ------------------------------------
 */
void silc_mgmtd_cfg_set_config_file(const silc_cstr filename)
{
	strcpy(s_silc_mgmtd_config_ctl.filename, filename);
	sprintf(s_silc_mgmtd_config_ctl.running_config_filename, "/dev/shm/is_mgmtd.conf.running.templet");
}

silc_cstr silc_mgmtd_cfg_get_config_filename()
{
	return s_silc_mgmtd_config_ctl.filename;
}

silc_cstr silc_mgmtd_cfg_get_running_config_filename()
{
	return s_silc_mgmtd_config_ctl.running_config_filename;
}

silc_bool silc_mgmtd_cfg_check_file_exist(const silc_cstr filename)
{
	struct stat st;

	if(stat(filename, &st) < 0)
		return silc_false;
	return silc_true;
}

silc_bool silc_mgmtd_cfg_config_file_exist()
{
	return silc_mgmtd_cfg_check_file_exist(s_silc_mgmtd_config_ctl.filename);
}

silc_bool silc_mgmtd_cfg_running_config_file_exist()
{
	return silc_mgmtd_cfg_check_file_exist(s_silc_mgmtd_config_ctl.running_config_filename);
}


/*
 * ---------------------------------------- SAVE CONFIGURATION -----------------------------------
 */
/*
 *  --- configuration file format ---
 * node_path,node_val_str
 */

int silc_mgmtd_cfg_write_node_to_file(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data)
{
	FILE* fd = (FILE*)cb_data;
	silc_cstr buff = s_silc_mgmtd_config_ctl.read_buff;
	char str_buf[200];
	int min_len;
	silc_cstr val_str = str_buf;

	if(p_db_node->type == MEMDB_NODE_TYPE_TEMPLATE ||
			p_db_node->type == MEMDB_NODE_TYPE_TEMPLATE_SUB ||
			p_db_node->type == MEMDB_NODE_TYPE_UNKNOWN ||
			p_db_node->type == MEMDB_NODE_TYPE_NULL ||
			p_db_node->type == MEMDB_NODE_TYPE_MAX)
		return 0;

	min_len = silc_mgmtd_var_get_val_str_min_len(&p_db_node->value);
	if(min_len > 200)
	{
		val_str = malloc(min_len);
		if(!val_str)
		{
			silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
			return -1;
		}
	}
	if(p_db_node->value.type == SILC_MGMTD_VAR_NULL || p_db_node->type == MEMDB_NODE_TYPE_DYNAMIC)
		sprintf(buff, "%s,\n", path);
	else
		sprintf(buff, "%s,%s\n", path, silc_mgmtd_var_to_str(&p_db_node->value, val_str, min_len));
	if(min_len > 200)
		free(val_str);
	if(fputs(buff, fd) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return -1;
	}
	return 0;
}

int silc_mgmtd_cfg_save_config_to_file_inner(const silc_cstr filename)
{
	int ret = 0;
	FILE* fd = 0;
	silc_mgmtd_node* p_node;
	silc_cstr buff = s_silc_mgmtd_config_ctl.read_buff;
	time_t cur_time;

	p_node = silc_mgmtd_memdb_find_node("/config");
	if(!p_node)
		goto ERROR_OUT;

	fd = fopen(filename, "w");
	if(!fd)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	// write configurations
	cur_time = time(NULL);
	sprintf(buff, "## this is a configuration file\n## latest update time is: %s\n", ctime(&cur_time));
	if(fputs(buff, fd) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	ret = silc_mgmtd_memdb_node_tree_traversal("/", p_node, silc_mgmtd_cfg_write_node_to_file, fd);
	if(ret != 0)
		goto ERROR_OUT;

	if(fclose(fd) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}
	return 0;

ERROR_OUT:
	if(fd)
		fclose(fd);
	return -1;
}

int silc_mgmtd_cfg_save_running_config_to_file(silc_cstr filename)
{
	return silc_mgmtd_cfg_save_config_to_file_inner(filename);
}

int silc_mgmtd_cfg_delete_config_from_file(const silc_cstr filename, const silc_cstr delete_path)
{
	FILE *wfd = 0, *rfd = 0;
	silc_cstr buff = s_silc_mgmtd_config_ctl.read_buff;
	char tmp_filename[20];
	char sys_cmd[100];

	sprintf(tmp_filename, "%s.tmp", filename);

	wfd = fopen(tmp_filename, "w");
	rfd = fopen(filename, "r");
	if(!wfd || !rfd)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	// write configurations
	while(1)
	{
		if(fgets(buff, 1024, rfd) == NULL)
			break;
		if(strncmp(buff, delete_path, strlen(delete_path)) == 0)
			continue;
		if(fputs(buff, wfd) < 0)
		{
			silc_mgmtd_err_set(MGMTD_ERR_CLIB, tmp_filename);
			goto ERROR_OUT;
		}
	}

	if(fclose(wfd) < 0 || fclose(rfd) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	sprintf(sys_cmd, "mv -f %s %s", tmp_filename, filename);
	system(sys_cmd);
	return 0;

ERROR_OUT:
	if(wfd)
		fclose(wfd);
	if(rfd)
		fclose(rfd);
	return -1;
}

int silc_mgmtd_cfg_add_config_into_file(const silc_cstr filename, silc_mgmtd_node *p_node)
{
	int ret = 0;
	FILE* fd = 0;
	char path[256];

	silc_mgmtd_memdb_get_node_path(p_node->p_parent, path);
	fd = fopen(filename, "a");
	if(!fd)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	// write configurations
	ret = silc_mgmtd_memdb_node_tree_traversal(path, p_node, silc_mgmtd_cfg_write_node_to_file, fd);
	if(ret != 0)
		goto ERROR_OUT;

	if(fclose(fd) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}
	return 0;

ERROR_OUT:
	if(fd)
		fclose(fd);
	return -1;
}

int silc_mgmtd_cfg_add_config_into_running_templet_file(silc_mgmtd_node* p_node)
{
	return silc_mgmtd_cfg_add_config_into_file(s_silc_mgmtd_config_ctl.running_config_filename, p_node);
}


int silc_mgmtd_cfg_save_config_to_file()
{
	char ext_cmd[256];
	char* conf = s_silc_mgmtd_config_ctl.filename;

	silc_mutex_lock(s_silc_mgmtd_config_ctl.write_lock);

	if(silc_mgmtd_cfg_save_config_to_file_inner("/dev/shm/config.save.tmp") != 0)
		goto ERROR_OUT;

	sprintf(ext_cmd, "mv -f /dev/shm/config.save.tmp %s; /usr/bin/sha1sum %s > %s.sha1sum; sync;",
			conf, conf, conf);
	if(silc_mgmtd_op_external_run(ext_cmd) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CFG_MV_FILE_FAILED, conf);
	}
	silc_mutex_unlock(s_silc_mgmtd_config_ctl.write_lock);
	SILC_LOG("Config file save done!");
	silc_mgmtd_memdb_set_config_saved();
	return 0;

ERROR_OUT:
	silc_mutex_unlock(s_silc_mgmtd_config_ctl.write_lock);
	return -1;
}

#ifndef SILC_MGMTD_LOCAL_DEBUG
#define SILC_MGMTD_CFG_INNER_PATH	"/config/inner"
#define SILC_MGMTD_CFG_EXTEND_PATH	"/config/extend"
#else
#define SILC_MGMTD_CFG_INNER_PATH	"./inner"
#define SILC_MGMTD_CFG_EXTEND_PATH	"./extend"
#endif
int silc_mgmtd_cfg_save_as_config_to_file(silc_cstr filename)
{
	char path[200];
	sprintf(path, SILC_MGMTD_CFG_INNER_PATH"/%s", filename);
	return silc_mgmtd_cfg_save_config_to_file_inner(path);
}

int silc_mgmtd_cfg_remove_config_file(silc_cstr filename)
{
	char cmd[200];
	sprintf(cmd, "rm -f "SILC_MGMTD_CFG_INNER_PATH"/%s", filename);
	if(silc_mgmtd_op_external_run(cmd) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CFG_MV_FILE_FAILED, filename);
		return -1;
	}
	return 0;
}

int silc_mgmtd_cfg_load_config_file(silc_cstr filename)
{
	char fullpath[64];
	char cmd[256];
	char* conf = s_silc_mgmtd_config_ctl.filename;

	sprintf(fullpath, SILC_MGMTD_CFG_INNER_PATH"/%s", filename);
	if(access(fullpath, F_OK) != 0)
	{
		return -1;
	}

	sprintf(cmd, "cp -f %s %s; /usr/bin/sha1sum %s > %s.sha1sum; sync;",
			fullpath, conf, conf, conf);
	if(silc_mgmtd_op_external_run(cmd) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CFG_MV_FILE_FAILED, filename);
		return -1;
	}

	return 0;
}

int silc_mgmtd_cfg_load_default_config_file()
{
	char cmd[200];

	// delete the config+backup file and load the default config from code in next boot
	sprintf(cmd, "rm -f %s* %s; sync;", s_silc_mgmtd_config_ctl.filename, s_silc_mgmtd_config_ctl.running_config_filename);
	if(silc_mgmtd_op_external_run(cmd) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CFG_MV_FILE_FAILED, s_silc_mgmtd_config_ctl.filename);
		return -1;
	}
	return 0;
}

int silc_mgmtd_cfg_get_config_file_list(silc_cstr buff, int* p_buff_len)
{
	return silc_mgmtd_if_exec_system_cmd("ls -l "SILC_MGMTD_CFG_INNER_PATH"|grep root|awk '{print $9}'", buff, p_buff_len, 1000, silc_false);
}

int silc_mgmtd_cfg_node_confirm_cb(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data)
{
	return silc_mgmtd_memdb_modify_node_confirm(p_db_node);
}

int silc_mgmtd_cfg_confirm_modify(silc_mgmtd_node* p_node)
{
	return silc_mgmtd_memdb_node_tree_traversal("", p_node, silc_mgmtd_cfg_node_confirm_cb, NULL);
}

int silc_mgmtd_cfg_node_clean_cb(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data)
{
	silc_mgmtd_var_clear(&p_db_node->tmp_value);
	return 0;
}

int silc_mgmtd_cfg_clean_modify(silc_mgmtd_node* p_node)
{
	return silc_mgmtd_memdb_node_tree_traversal("", p_node, silc_mgmtd_cfg_node_clean_cb, NULL);
}

/*
 * ------------------------------------ READ CONFIGURATION ----------------------------------
 */
int silc_mgmtd_cfg_add_node(const silc_cstr path_str, silc_cstr val_str)
{
	silc_mgmtd_node* p_src_node;
	silc_mgmtd_node* p_dst_node;
	silc_mgmtd_node* p_parent = silc_mgmtd_memdb_find_parent(path_str);

	if(!p_parent)
		return 0;

	if(silc_list_empty(&p_parent->sub_node_list)) // unknown node, need update
	{
UNKNOWN_NODE:
		SILC_LOG("Warning: unknown node %s!!\n", path_str);
		p_dst_node = silc_mgmtd_memdb_create_node(silc_mgmtd_if_path_get_last_name(path_str),
				SILC_MGMTD_IF_LEVEL_READONLY, MEMDB_NODE_TYPE_UNKNOWN,
				SILC_MGMTD_VAR_STRING, val_str, NULL, 0);
		if(!p_dst_node)
			return -1;
		silc_mgmtd_memdb_insert_node(p_dst_node, p_parent);
		return 0;
	}

	p_src_node = silc_list_entry(p_parent->sub_node_list.next, silc_mgmtd_node, node);
	if(p_src_node->type != MEMDB_NODE_TYPE_TEMPLATE)
		goto UNKNOWN_NODE;

	p_dst_node = silc_mgmtd_memdb_clone_node(p_src_node);
	if(!p_dst_node)
		return -1;

	if(silc_mgmtd_var_set_by_str(&p_dst_node->value, silc_mgmtd_if_path_get_last_name(path_str)) != 0)
	{
		silc_mgmtd_memdb_delete_node(p_dst_node);
		return -1;
	}

	silc_mgmtd_memdb_insert_node(p_dst_node, p_parent);

	return 0;
}

int silc_mgmtd_cfg_modify_node_from_line(silc_cstr line, silc_bool def_config)
{
	silc_mgmtd_node* p_node = NULL;
	silc_cstr path_str, val_str;

	path_str = line;
	val_str = strstr(line, ",");
	if(!val_str)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CFG_INVALID_NODESTR, line);
		return -1;
	}
	val_str[0] = 0;
	val_str++;

	if(def_config)
	{
		p_node = silc_mgmtd_memdb_find_init_node(path_str);
	}
	if(!p_node)
	{
		p_node = silc_mgmtd_memdb_find_node(path_str);
	}
	if(!p_node)
	{
		//insert
		if(def_config)
		{
			silc_cstr name = silc_mgmtd_memdb_path_get_last_name(path_str);
			silc_mgmtd_node* p_parent = silc_mgmtd_memdb_find_parent(path_str);
			if(!p_parent)
			{
				SILC_ERR("Fail to find parent for path %s!", path_str);
				return -1;
			}
			if(silc_mgmtd_op_add_node(p_parent, path_str, name) != 0)
			{
				SILC_ERR("silc_mgmtd_op_do_add_single %s error!", path_str);
				return -1;
			}
		}
		else if(silc_mgmtd_cfg_add_node(path_str, val_str) != 0)
			return -1;
	}
	else
	{
		if(def_config)
		{
			if(p_node->type == MEMDB_NODE_TYPE_DYNAMIC)
			{
				SILC_LOG("%s is an existed dynamic node, ignore it", path_str);
				return 0;
			}
			if(p_node->value.type != SILC_MGMTD_VAR_NULL &&
					 silc_mgmtd_var_set_by_str(&p_node->value, val_str) < 0)
			{
				silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
				return -1;
			}
			silc_mgmtd_var_copy(&p_node->value, &p_node->tmp_value);
		}
		else if(silc_mgmtd_memdb_modify_node(p_node, val_str) < 0)
			return -1;
	}

	return 0;
}

int silc_mgmtd_cfg_read_config_from_file_core(silc_cstr filename, silc_bool def_config)
{
	FILE* fd;
	silc_cstr line;
	silc_cstr buff = s_silc_mgmtd_config_ctl.read_buff;
	static char node_str[SILC_MGMTD_CONFIG_READ_LINE_MAX_LEN];
	int len;

	fd = fopen(filename, "r");
	if(!fd)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return -1;
	}

	node_str[0] = '\0';
	line = fgets(buff, SILC_MGMTD_CONFIG_READ_LINE_MAX_LEN, fd);
	while(line)
	{
		if (strncmp(line, "/config", 7) == 0)
		{
			len = strlen(node_str);
			if(len > 0 && node_str[len-1] == '\n')
				node_str[len-1] = '\0';
			if (node_str[0] > '\0')
			{
				if(silc_mgmtd_cfg_modify_node_from_line(node_str, def_config) < 0)
				{
					SILC_ERR("Invalid node line: %s", node_str);
					goto ERROR_OUT;
				}
				node_str[0] = '\0';
			}
			strcpy(node_str, line);
		}
		else if(node_str[0] > '\0')
		{
			strcat(node_str, line);
		}

		line = fgets(buff, SILC_MGMTD_CONFIG_READ_LINE_MAX_LEN, fd);
	}
	if (node_str[0] > '\0')
	{
		len = strlen(node_str);
		if(len > 0 && node_str[len-1] == '\n')
			node_str[len-1] = '\0';
		if(silc_mgmtd_cfg_modify_node_from_line(node_str, def_config) < 0)
		{
			SILC_ERR("Invalid node line: %s", node_str);
			goto ERROR_OUT;
		}
	}

	fclose(fd);
	SILC_TRACE("[%s] read done!", __func__);
	return 0;

ERROR_OUT:
	fclose(fd);
	return -1;
}

int silc_mgmtd_cfg_read_config_from_file()
{
	int ret = 0;

	silc_mutex_lock(s_silc_mgmtd_config_ctl.write_lock);

	if(0 != silc_mgmtd_cfg_read_config_from_file_core(s_silc_mgmtd_config_ctl.filename, silc_false))
		ret = -1;

	silc_mutex_unlock(s_silc_mgmtd_config_ctl.write_lock);
	return ret;
}

static char silc_mgmtd_default_admin[64]={0};
static char silc_mgmtd_default_admin_shadow[256]={0};

void silc_mgmtd_set_default_admin(silc_cstr val)
{
	if(strlen(val) >= 64)
	{
		SILC_ERR("Default admin name %s too long!", val);
		return;
	}
	strcpy(silc_mgmtd_default_admin, val);
}

silc_cstr silc_mgmtd_get_default_admin()
{
	if(silc_mgmtd_default_admin[0])
		return silc_mgmtd_default_admin;
	return NULL;
}

void silc_mgmtd_set_default_admin_shadow(silc_cstr val)
{
	if(strlen(val) >= 256)
	{
		SILC_ERR("Default admin name %s too long!", val);
		return;
	}
	strcpy(silc_mgmtd_default_admin_shadow, val);
}

silc_cstr silc_mgmtd_get_default_admin_shadow()
{
	if(silc_mgmtd_default_admin_shadow[0])
		return silc_mgmtd_default_admin_shadow;
	return NULL;
}

void silc_mgmtd_add_product_default_admin()
{
	silc_mgmtd_node *p_unix_user, *p_admin_node;
	silc_mgmtd_if_req* p_sim_req = NULL;
	silc_cstr admin, shadow;
	char val_str[64], path_str[256];

	// add default admin if necessary
	admin = silc_mgmtd_get_default_admin();
	if(!admin)
	{
		SILC_ERR("Default admin not set");
		return;
	}
	p_unix_user = silc_mgmtd_memdb_find_node("/config/unix/user");
	if(!p_unix_user)
		return;
	if(silc_mgmtd_memdb_find_sub_value(p_unix_user, admin))
	{
		return;
	}

	sprintf(path_str, "/config/unix/user/%s", admin);
	p_sim_req = silc_mgmtd_if_req_create(path_str, SILC_MGMTD_IF_REQ_ADD, admin);
	if(!p_sim_req)
		goto ERROR;
	shadow = silc_mgmtd_get_default_admin_shadow();
	if(!shadow)
		goto ERROR;
	if(!silc_mgmtd_if_req_add_node_ext(p_sim_req->p_node_root, "shadow", shadow))
		goto ERROR;
	sprintf(val_str, "%d", SILC_MGMTD_IF_LEVEL_ADMIN);
	if(!silc_mgmtd_if_req_add_node_ext(p_sim_req->p_node_root, "privilege", val_str))
		goto ERROR;
	if(!silc_mgmtd_if_req_add_node_ext(p_sim_req->p_node_root, "full-name", "Default administrator"))
		goto ERROR;
	if(silc_mgmtd_op_do_add_single(p_unix_user, p_sim_req->p_node_root, NULL, silc_true) != 0)
		goto ERROR;
	p_admin_node = silc_mgmtd_memdb_find_node(path_str);
	if(!p_admin_node)
		goto ERROR;

	silc_mgmtd_if_req_delete(p_sim_req);
	return;
ERROR:
	if(p_sim_req)
		silc_mgmtd_if_req_delete(p_sim_req);
	return;
}

//this should be called after loading OEM config to read OEM default admin and pass
void silc_mgmtd_add_default_admin()
{
	silc_mgmtd_node *p_unix_user, *p_node, *p_sub_node;
	p_unix_user = silc_mgmtd_memdb_find_node("/config/unix/user");
	if(!p_unix_user)
		return;
	silc_list_for_each_entry(p_node, &p_unix_user->sub_node_list, node)
	{
		if(p_node->type == MEMDB_NODE_TYPE_TEMPLATE)
			continue;
		p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "privilege");
		if(SILC_MGMTD_IF_LEVEL_ADMIN == p_sub_node->value.val.uint32_val)
		{
			//find the 1st admin user as default admin
			silc_mgmtd_set_default_admin(p_node->value.val.string_val);
			p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "shadow");
			silc_mgmtd_set_default_admin_shadow(p_sub_node->value.val.string_val);
			SILC_LOG("Default admin is %s", silc_mgmtd_get_default_admin());
			return;
		}
	}
	//if not found, set it to product default
	silc_mgmtd_add_product_default_admin();
}

void silc_mgmtd_cfg_add_dft_admin_to_running()
{
	char* admin = silc_mgmtd_get_default_admin();
	char path[128];
	silc_mgmtd_node *p_admin_node;

	if(!admin)
	{
		SILC_LOG("Default admin not found!");
		return;
	}
	sprintf(path, "/config/unix/user/%s", admin);
	p_admin_node = silc_mgmtd_memdb_find_node(path);
	if(!p_admin_node)
	{
		SILC_LOG("Default admin node not found!");
		return;
	}
	if(silc_mgmtd_cfg_add_config_into_running_templet_file(p_admin_node) != 0)
		SILC_ERR("Fail to add default admin to running config!");
}

static silc_cstr silc_mgmtd_oem_devname = NULL;


silc_cstr silc_mgmtd_get_devname()
{
	//if there's an oem name, return it. otherwise return default product name
	if(silc_mgmtd_oem_devname)
		return silc_mgmtd_oem_devname;
	else
		return silc_mgmtd_memdb_get_product_info()->product_name;
}

int silc_mgmtd_get_ver_from_file(silc_cstr filename, silc_cstr version)
{
	char str[256];
	FILE* fp;

	version[0] = 0;
	fp = fopen(filename, "r");
	if (!fp)
		return -1;
	fgets(str, 256, fp);
	// fmt: ##VERSION_STR##
	sscanf(str, "##%[^#]", version);
	fclose(fp);
	return 0;
}

void silc_mgmtd_update_ts(silc_cstr module, void* conn_entry)
{
	char cmd[128];
	// prevent trigger netconf sync
	if(conn_entry && silc_mgmtd_if_server_get_conn_type(conn_entry) == SILC_MGMTD_IF_CLIENT_NETCONF)
		return;
	sprintf(cmd, "echo '1' > /tmp/.stamp_%s", module);
	system(cmd);
}

int silc_mgmtd_cfg_modify_default_node(silc_cstr line)
{
	silc_mgmtd_node* p_node = NULL;
	silc_cstr_array* items;
	silc_cstr path_str, val_str;
	silc_mgmtd_if_req* p_sim_req = NULL;

	items = silc_cstr_split(line, ",");
	if(!items)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CFG_INVALID_NODESTR, line);
		goto ERROR_OUT;
	}

	path_str = silc_cstr_array_get_quick(items, 0);
	p_node = silc_mgmtd_memdb_find_init_node(path_str);
	val_str = silc_cstr_array_get_quick(items, 1);
	if(!val_str)
		val_str = "";

	if(!p_node)
	{
		p_node = silc_mgmtd_memdb_find_node(path_str);
	}
	if(p_node)
	{
		if(p_node->value.type != SILC_MGMTD_VAR_NULL &&
				 silc_mgmtd_var_set_by_str(&p_node->value, val_str) < 0)
		{
			silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
			goto ERROR_OUT;
		}
		silc_mgmtd_var_copy(&p_node->value, &p_node->tmp_value);
	}
	else
	{
		//insert
		silc_cstr name = silc_mgmtd_memdb_path_get_last_name(path_str);
		silc_mgmtd_node* p_parent = silc_mgmtd_memdb_find_parent(path_str);
#if 0
		p_sim_req = silc_mgmtd_if_req_create(path_str, SILC_MGMTD_IF_REQ_ADD, name);
		if(!p_sim_req)
		{
			SILC_ERR("silc_mgmtd_if_req_create %s error!", path_str);
			goto ERROR_OUT;
		}
		if(silc_mgmtd_op_do_add_single(p_parent, p_sim_req->p_node_root, NULL, silc_false) != 0)
#else
		if(silc_mgmtd_op_add_node(p_parent, path_str, name) != 0)
#endif
		{
			SILC_ERR("silc_mgmtd_op_do_add_single %s error!", path_str);
			goto ERROR_OUT;
		}
	}
	SILC_TRACE("[%s] set node %s, %s", __func__, path_str, val_str);
	silc_cstr_array_destroy(items);
	return 0;

ERROR_OUT:
	if(items)
		silc_cstr_array_destroy(items);
	if(p_sim_req)
		silc_mgmtd_if_req_delete(p_sim_req);
	return -1;
}

int silc_mgmtd_cfg_read_default_config(char* cfg_path)
{
	FILE* fd;
	silc_cstr line;
	silc_cstr buff = s_silc_mgmtd_config_ctl.read_buff;
	int len;

	if(access(cfg_path, F_OK))
	{
		SILC_LOG("Default config %s doesn't exist!", cfg_path);
		return 0;
	}
	fd = fopen(cfg_path, "r");
	if(!fd)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return -1;
	}

	line = fgets(buff, SILC_MGMTD_CONFIG_READ_LINE_MAX_LEN, fd);
	while(line)
	{
		if (strncmp(line, "/config", 7) == 0)
		{
			len = strlen(line);
			if(len > 0 && line[len-1] == '\n')
				line[len-1] = '\0';
			if(silc_mgmtd_cfg_modify_default_node(line) < 0)
				goto ERROR_OUT;
		}
		line = fgets(buff, SILC_MGMTD_CONFIG_READ_LINE_MAX_LEN, fd);
	}

	fclose(fd);
	SILC_TRACE("[%s] read default config %s done!", __func__, cfg_path);
	return 0;
ERROR_OUT:
	SILC_ERR("[%s] read default config %s failed!", __func__, cfg_path);
	fclose(fd);
	return -1;
}

#define PROD_DEFCONF_DIR	"/etc/prod_defconfig"
#define PROD_VENDOR_DIR		"/etc/prod_defconfig/vendor_config"

int silc_mgmtd_cfg_read_product_default_config()
{
	char cfg_path[200];

	sprintf(cfg_path, PROD_DEFCONF_DIR"/%s.default_config",
			SILC_MGMTD_PRODUCT_NAME);
	return silc_mgmtd_cfg_read_config_from_file_core(cfg_path, silc_true);
}

int silc_mgmtd_cfg_read_oem_default_config()
{
	char cfg_path[200];

	sprintf(cfg_path, PROD_VENDOR_DIR"/%s/%s.default_config",
			silc_mgmtd_if_get_vendor_name(), silc_mgmtd_if_get_vendor_name());
	return silc_mgmtd_cfg_read_config_from_file_core(cfg_path, silc_true);
}

int silc_mgmtd_cfg_read_oem_devname()
{
	char path[200];
	silc_cstr buff;
	uint32_t len;

	sprintf(path, PROD_VENDOR_DIR"/%s/%s.devname",
			silc_mgmtd_if_get_vendor_name(), silc_mgmtd_if_get_vendor_name());
	if(access(path, F_OK))
		return 0;
	if(0 != silc_file_read_all(path, &buff, &len))
	{
		SILC_ERR("Fail to read %s", path);
		return -1;
	}
	if(buff[len-1] == '\n' || buff[len-1] == '\r')
	{
		len--;
	}
	buff[len] = 0;

	//free old just in case this is called multiple times
	if(silc_mgmtd_oem_devname)
		free(silc_mgmtd_oem_devname);
	silc_mgmtd_oem_devname = buff;
	return 0;
}

int silc_mgmtd_cfg_apply_oem()
{
	if(NULL == silc_mgmtd_if_get_vendor_name())
	{
		SILC_LOG("Use SILICOM default!");
		return 0;
	}
	silc_mgmtd_cfg_read_oem_devname();
	silc_mgmtd_cfg_read_oem_default_config();
	return 0;
}

int silc_mgmtd_cfg_apply_product_config()
{
	silc_mgmtd_cfg_read_product_default_config();
	silc_mgmtd_cfg_apply_oem();
	silc_mgmtd_add_default_admin();
	return 0;
}

/*
 * -------------------------------------- ENABLE CONFIGURATION -----------------------------------
 */
int silc_mgmtd_cfg_enable_modify_node(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data)
{
//	printf("enable path:%s, node:%s, type:%d\n", path, p_db_node->name, p_db_node->type);

	if(p_db_node->type == MEMDB_NODE_TYPE_TEMPLATE ||
			p_db_node->type == MEMDB_NODE_TYPE_TEMPLATE_SUB)
		return 0;

	if(p_db_node->type == MEMDB_NODE_TYPE_DYNAMIC) // enable add new node tree
	{
		SILC_TRACE("[%s] Enable new %s!", __func__, path);
		return silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_ADD, p_db_node, p_db_node->do_ops_timeout, NULL);
	}

	if(p_db_node->tmp_value.type == SILC_MGMTD_VAR_NULL) // there is no modification
		return 0;

	if(p_db_node->type == MEMDB_NODE_TYPE_DYNAMIC_SUB) // do not call cb, already handled by add
		return 0;

	// static node modified enable
	return silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_MODIFY, p_db_node, p_db_node->do_ops_timeout, NULL);
}

int silc_mgmtd_cfg_enable_config()
{
	silc_mgmtd_node* p_node;

	p_node = silc_mgmtd_memdb_find_node("/config");
	if(!p_node)
		return -1;

	return silc_mgmtd_memdb_node_tree_traversal("/", p_node, silc_mgmtd_cfg_enable_modify_node, NULL);
}

int silc_mgmtd_cfg_backup_config()
{
	static char cmd[SILC_MGMTD_CONFIG_FILE_NAME_MAX_LEN*4];
	char* conf = s_silc_mgmtd_config_ctl.filename;
	char bak[SILC_MGMTD_CONFIG_FILE_NAME_MAX_LEN];

	sprintf(bak, "%s.backup", conf);
	sprintf(cmd, "if [ ! -e \"%s\" ] || [ \"$(/usr/bin/diff %s %s)\" != \"\" ]; "
			"then cp %s %s; /usr/bin/sha1sum %s > %s.sha1sum; sync; fi",
			bak, conf, bak, conf, bak, bak, bak);
	if(silc_mgmtd_op_external_run(cmd) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CFG_MV_FILE_FAILED, conf);
		return -1;
	}

	return 0;
}

int silc_mgmtd_cfg_load_config_from_file()
{
	int ret = 0;
	silc_mgmtd_node* p_node = NULL;
	silc_mgmtd_node* p_cfg_root = NULL;
	silc_bool is_ver_change = silc_false;

	p_cfg_root = silc_mgmtd_memdb_find_node("/config");
	if(!p_cfg_root)
		return -1;

	silc_mgmtd_cfg_apply_product_config();

	if(!silc_mgmtd_cfg_config_file_exist() &&
			silc_mgmtd_cfg_save_config_to_file() < 0)
		return -1;

	if(!silc_mgmtd_cfg_running_config_file_exist())
	{
		if(silc_mgmtd_memdb_get_product_info()->default_node_add_func)
		{
			if(silc_mgmtd_memdb_get_product_info()->default_node_add_func() != 0)
				return -1;
		}
		ret = silc_mgmtd_cfg_save_config_to_file_inner(s_silc_mgmtd_config_ctl.running_config_filename);

		if(silc_mgmtd_memdb_get_product_info()->default_node_add_func)
		{
			silc_mgmtd_memdb_get_product_info()->default_node_del_func();
		}
		if(ret != 0)
			return -1;
	}

	if(silc_mgmtd_cfg_read_config_from_file() < 0)
		return -1;

	// check version
	silc_list_for_each_entry(p_node, &p_cfg_root->sub_node_list, node)
	{
		if(p_node->value.type != SILC_MGMTD_VAR_UINT32 ||
				p_node->tmp_value.type != SILC_MGMTD_VAR_UINT32)
			continue;
		if(!silc_mgmtd_var_equal(&p_node->value, &p_node->tmp_value))
		{
			SILC_LOG("[%s] Module %s upgrade from %d to %d.\n",
					__func__, p_node->name, p_node->tmp_value.val.uint32_val, p_node->value.val.uint32_val);
			is_ver_change = silc_true;
			if((ret=silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_UPGRADE, p_node, p_node->do_ops_timeout, NULL)) != 0)
				return ret;
			silc_mgmtd_var_clear(&p_node->tmp_value);
		}
	}

	silc_mgmtd_cfg_confirm_modify(p_cfg_root);
	if(silc_mgmtd_cfg_enable_config() < 0)
		return -1;

	silc_mgmtd_cfg_clean_modify(p_cfg_root);

	if(is_ver_change)
	{
		if(silc_mgmtd_cfg_save_config_to_file() < 0)
			return -1;
	}

	silc_mgmtd_cfg_backup_config();

	return 0;
}

/*
 * --------------------------------------- INIT CONFIG -------------------------------------
 */
void silc_mgmtd_cfg_init(const silc_cstr cfg_filename)
{
	silc_mgmtd_cfg_set_config_file(cfg_filename);
	s_silc_mgmtd_config_ctl.write_lock = silc_mutex_create();
	system("mkdir -p "SILC_MGMTD_CFG_INNER_PATH);
	system("mkdir -p "SILC_MGMTD_CFG_EXTEND_PATH);
}
