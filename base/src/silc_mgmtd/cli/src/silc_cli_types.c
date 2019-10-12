
#include "silc_cli_types.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_terminal.h"
#include "silc_mgmtd_interface.h"


static silc_cli_mode s_silc_cli_mode_root;
static silc_cli_mode* s_p_silc_cli_curr_modes;
static char s_silc_cli_device_name[256] = "DevName";

static silc_cli_token_overlay silc_cli_token_overlays[SILC_CLI_TOKEN_OVERLAY_MAX];
static int silc_cli_token_overlay_count = -1;

#define OV_BUF_SIZE (SILC_CLI_PATH_MAX + 8)

/*
 * ---------------------------- TOKEN -------------------------------
 */
silc_cli_token* silc_cli_token_find_parent_inner(const silc_cstr cmd_path, silc_cli_token* p_root)
{
	int name_len;
	char name[100];
	silc_cli_token* p_token;
	silc_cstr sub_path, path;

	path = cmd_path;

	if(strlen(cmd_path) == 0)
		return p_root;

	sub_path = strstr(path, " ");
	if(!sub_path)
		return p_root;
	else
		sub_path += 1; // skip space
	name_len = sub_path - path - 1;
	strncpy(name, path, name_len);
	name[name_len] = '\0';

	silc_list_for_each_entry(p_token, &p_root->sub_token_list, node)
	{
		if(strcmp(name, p_token->name) == 0)
			return silc_cli_token_find_parent_inner(sub_path, p_token);
	}
	silc_cli_err_set(CLI_ERR_TYPES_TOKEN_NOTEXIST);
	return NULL;
}

silc_cli_token* silc_cli_token_find_parent(const silc_cstr token_path)
{
	return silc_cli_token_find_parent_inner(token_path, &s_silc_cli_mode_root);
}

silc_cli_token* silc_cli_token_find_comm(const silc_cstr token_path, silc_cli_token* p_root)
{
	int name_len;
	char name[100];
	silc_cli_token* p_token;
	silc_cstr sub_path, path;

	path = token_path;

	if(strlen(path) == 0)
		return p_root;

	sub_path = strstr(path, " ");
	if(!sub_path)
	{
		sub_path = path + strlen(path);
		name_len = sub_path - path;
	}
	else
	{
		sub_path += 1;
		name_len = sub_path - path - 1;
	}
	if(name_len >= 100)
	{
		silc_cli_err_set(CLI_ERR_TYPES_TOKEN_NOTEXIST);
		return NULL;
	}
	strncpy(name, path, name_len);
	name[name_len] = '\0';

	silc_list_for_each_entry(p_token, &p_root->sub_token_list, node)
	{
		if(strcmp(name, p_token->name) == 0)
			return silc_cli_token_find_comm(sub_path, p_token);
	}
	silc_cli_err_set(CLI_ERR_TYPES_TOKEN_NOTEXIST);
	return NULL;
}

silc_cli_token* silc_cli_token_find(const silc_cstr token_path)
{
	return silc_cli_token_find_comm(token_path, &s_silc_cli_mode_root);
}

silc_cli_token* silc_cli_token_find_from_list(silc_list* p_token_list, const silc_cstr name)
{
	silc_cli_token* p_token;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(name, p_token->name) == 0)
			return p_token;
	}
	return NULL;
}

silc_cstr silc_cli_token_path_get_last_name(const silc_cstr path)
{
	int loop;
	int path_len = strlen(path);

	for(loop=path_len; loop>=0; loop--)
	{
		if(path[loop] == ' ')
			return path+loop+1;
	}
	return path;
}

#define silc_cli_token_copy_str_info(token_field, info_field)	\
	do {\
		if(info_field) \
		{ \
			token_field = cur_pos;	\
			cur_pos += strlen(info_field) + 1; \
			strcpy(token_field, info_field); \
		}\
	}while(0)

static int silc_cli_overlay_add(const char *path, const char *set)
{
	int index = silc_cli_token_overlay_count;
	int i;

	for(i = 0; i < silc_cli_token_overlay_count; i++)
	{
		if(!strcmp(silc_cli_token_overlays[i].path, path))
		{
			index = i;
			break;
		}
	}

	silc_cli_token_overlays[index].setup = atoi(set);

	if(index == silc_cli_token_overlay_count)
	{
		// new entry
		strcpy(silc_cli_token_overlays[index].path, path);
		silc_cli_token_overlay_count++;
	}

	return 0;
}

static int silc_cli_cfg_parse(silc_cstr filename)
{
	FILE *fp;
	char buf[OV_BUF_SIZE];
	char *path, *val, *saveptr;

	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		silc_cli_print("Error! Fail to read cli cfg file: %s\n", filename);
		return 0;
	}

	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if(buf[0] == '\n' || buf[0] == '#') // skip empty line
		  continue;

		path = strtok_r(buf, "=", &saveptr);
		if(path == NULL)
		{
			silc_cli_print("Warning! Invalid cli cfg line: %s\n", buf);
			continue;
		}

		val = strtok_r(NULL, "=", &saveptr);
		if(val == NULL)
		{
			silc_cli_print("Warning! Invalid cli cfg line: %s\n", buf);
			continue;
		}

		silc_cli_overlay_add(path, val);
		if(silc_cli_token_overlay_count >= SILC_CLI_TOKEN_OVERLAY_MAX)
		{
			silc_cli_print("Warning! Exceed cli cfg table size\n");
			break;
		}
	}

	fclose(fp);
	return 0;
}

static int silc_cli_cfg_load(void)
{
	char cfgfile[256];
	char* vendor = silc_mgmtd_if_get_vendor_name();

	// read product cli cfg
	sprintf(cfgfile, "/etc/prod_defconfig/%s.cli_config", SILC_MGMTD_PRODUCT_NAME);
	if(access(cfgfile, F_OK) == 0)
		silc_cli_cfg_parse(cfgfile);

	// read vendor cli cfg
	if(vendor)
	{
		sprintf(cfgfile, "/etc/prod_defconfig/vendor_config/%s/%s.cli_config", vendor, vendor);
		if(access(cfgfile, F_OK) == 0)
			silc_cli_cfg_parse(cfgfile);
	}

	return 0;
}

static int silc_cli_overlay_apply(silc_cli_token_info *p_info)
{
	int i;

	if(silc_cli_token_overlay_count < 0)
	{
		silc_cli_token_overlay_count = 0;
		silc_cli_cfg_load();
	}

	for(i = 0; i < silc_cli_token_overlay_count; i++)
	{
		if (strcmp(p_info->path, silc_cli_token_overlays[i].path) == 0)
			return silc_cli_token_overlays[i].setup;
	}

	return 1;
}

int silc_cli_token_add(silc_cli_token_info* p_info)
{
	silc_cli_token* p_parent;
	silc_cli_token* p_token;
	silc_cstr cur_pos;
	silc_cstr name = silc_cli_token_path_get_last_name(p_info->path);
	int len = sizeof(silc_cli_token) + strlen(name) +
			strlen(p_info->comment) + strlen(p_info->val_help) + 3 + p_info->dync_buff_len;

	if(silc_cli_overlay_apply(p_info) == 0)
	{
		//printf(">>> deny cli token %s\n", p_info->path);
		return 0;
	}

	p_parent = silc_cli_token_find_parent(p_info->path);
	if(!p_parent)
		return 0;

	if(p_info->map_name)
		len += strlen(p_info->map_name) + 1;
	if(p_info->map_val)
		len += strlen(p_info->map_val) + 1;
	p_token = malloc(len);
	if(!p_token)
	{
		silc_cli_err_set(CLI_ERR_SYSTEM);
		return -1;
	}

	memset(p_token, 0, len);
	p_token->val_str = NULL;
	p_token->p_parent = p_parent;
	silc_list_init(&p_token->err_list);
	silc_list_init(&p_token->sub_token_list);
	p_token->cmd_func = p_info->do_func;
	p_token->dync_func = p_info->dync_func;
	p_token->dync_val_str_buf_len = p_info->dync_buff_len;
	p_token->dync_func_data_idx = p_info->dync_data_idx;
	p_token->sub_mode = p_info->sub_token_mode;
	p_token->type = p_info->token_type;
	p_token->hidden = p_info->hidden;
	p_token->name = (silc_cstr)(p_token+1);
	strcpy(p_token->name, name);
	if(p_token->type == TOKEN_TYPE_STATIC)
		p_token->val_str = p_token->name;

	cur_pos = p_token->name + strlen(name) + 1;
	silc_cli_token_copy_str_info(p_token->comment, p_info->comment);
	silc_cli_token_copy_str_info(p_token->val_help, p_info->val_help);
	silc_cli_token_copy_str_info(p_token->map_name, p_info->map_name);
	silc_cli_token_copy_str_info(p_token->map_val, p_info->map_val);
	p_token->dync_val_str_buf = cur_pos;
	silc_list_add_tail(&p_token->node, &p_parent->sub_token_list);
	return 0;
}

void silc_cli_token_tree_show(silc_cli_token *p_token, int level)
{
	int loop;
	silc_cli_token *p_sub_token;

	for(loop=0;loop<level;loop++)
		printf("    ");
	printf("%s\n", p_token->name);
	silc_list_for_each_entry(p_sub_token, &p_token->sub_token_list, node)
	{
		silc_cli_token_tree_show(p_sub_token, level+1);
	}
}

void silc_cli_token_show_all()
{
	silc_cli_token *p_sub_token;

	silc_list_for_each_entry(p_sub_token, &s_silc_cli_mode_root.sub_token_list, node)
		silc_cli_token_tree_show(p_sub_token, 0);
}


int silc_cli_token_get_list_len(silc_list* p_token_list)
{
	int len = 0;
	silc_list_node* p_node;

	silc_list_for_each(p_node, p_token_list)
	{
		len++;
	}
	return len;
}

/*
 * -------------------------- MODE -------------------------------
 */
void silc_cli_mode_list_init()
{
	silc_list_init(&s_silc_cli_mode_root.sub_token_list);
	silc_list_init(&s_silc_cli_mode_root.err_list);
	s_silc_cli_mode_root.p_parent = NULL;
}

int silc_cli_mode_add(const silc_cstr name, const silc_cstr prompt)
{
	silc_cli_token_info info;

	memset(&info, 0, sizeof(info));
	info.path = name;
	info.comment = prompt;
	info.token_type = TOKEN_TYPE_MODE;
	info.sub_token_mode = TOKEN_MODE_SINGLE;
	info.val_help = "";
	return silc_cli_token_add(&info);
}

silc_cli_mode* silc_cli_mode_find(const silc_cstr name)
{
	return silc_cli_token_find(name);
}

int silc_cli_mode_curr_set(const silc_cstr name)
{
	silc_cli_mode* p_mode = silc_cli_mode_find(name);

	if(!p_mode)
		return -1;

	s_p_silc_cli_curr_modes = p_mode;
	return 0;
}

silc_cli_mode* silc_cli_mode_curr_get()
{
	if(!s_p_silc_cli_curr_modes)
		s_p_silc_cli_curr_modes = silc_list_entry(s_silc_cli_mode_root.sub_token_list.next, silc_cli_mode, node);
	return s_p_silc_cli_curr_modes;
}

silc_cstr silc_cli_mode_curr_name_get()
{
	return silc_cli_mode_curr_get()->name;
}

silc_cstr silc_cli_mode_get_curr_prompt()
{
	static char prompt[100];
	sprintf(prompt, "%s%s", silc_cli_mode_get_dev_name(), silc_cli_mode_curr_get()->comment);
	return prompt;
}

int silc_cli_mode_set_dev_name()
{
	//return silc_cli_get_mgmtd_dev_name(s_silc_cli_device_name);
	return gethostname(s_silc_cli_device_name, 100);
}

const silc_cstr silc_cli_mode_get_dev_name()
{
	return s_silc_cli_device_name;
}


