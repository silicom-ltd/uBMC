/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"

#define LOCAL_IMAGE_PATH		"/var/images"
#define CMD_BUFFER_SIZE			512

int is_cli_show_images_storage();

int is_cli_upload_image(silc_list* p_token_list, silc_cli_token* p_l1_token)
{
	silc_cli_token* p_l2_token;
	char distfile[CMD_BUFFER_SIZE];
	silc_cstr url = p_l1_token->val_str;

	p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
	if(p_l2_token && strcmp(p_l2_token->name, "as") == 0)
	{
		snprintf(distfile, CMD_BUFFER_SIZE, "%s/%s", LOCAL_IMAGE_PATH, p_l2_token->val_str);
	}
	else if(strncmp(url, "scp://", strlen("scp://")) == 0)
	{
		//sprintf(distfile, "%s/cdrom_%u.iso", LOCAL_IMAGE_PATH , (uint32_t)time(NULL));
		sprintf(distfile, "%s/", LOCAL_IMAGE_PATH);
	}
	else
	{
		snprintf(distfile, CMD_BUFFER_SIZE, "%s/%s", LOCAL_IMAGE_PATH, basename(url));
	}

	if(0 != silc_cli_upload_file(url, NULL, NULL, distfile))
		return -1;

	is_cli_show_images_storage();
	return 0;
}

int is_cli_cmd_image_config(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;
	silc_cli_token *p_token, *p_l1_token;

	memset(&req_info, 0, sizeof(req_info));
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
			continue;

		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "images") == 0)
		{
			if(!p_l1_token)
			{
				silc_cli_err_cmd_set_invalid_cmd();
				return -1;
			}
			if(strcmp(p_l1_token->name, "upload") == 0)
			{
				return is_cli_upload_image(p_token_list, p_l1_token);
			}
			silc_cli_err_cmd_set_invalid_cmd();
			return -1;
		}
	}

	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

