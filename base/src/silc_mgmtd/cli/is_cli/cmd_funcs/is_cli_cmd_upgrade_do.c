/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"

#define IS_UPGRADE_TOOL		"/usr/sbin/is_upgrade"
#define IS_UPGRADE_IMG		"/var/log/tmp/is_upgrade_pkg.img"
#define IS_UPGRADE_STATUS	"/tmp/is_upgrade.status"
#define IS_UPGRADE_LOG		"/tmp/is_upgrade.log"
#define IS_UPGRADE_PIDFILE	"/tmp/is_upgrade.pid"
#define IS_UPGRADE_PATH		"/tmp/upgrade/is_upgrade_path"
#define LOCAL_IMAGE_PATH	"/var/images"

int is_cli_upload_file(silc_cstr url, silc_cstr user, silc_cstr passwd, silc_cstr filename);

int is_cli_cmd_upload_url_err()
{
	silc_cli_err_cmd_set_err_info("unknown URL");
	return -1;
}

int is_cli_cmd_verify_software(silc_cstr filename)
{
	int len = 512;
	char buf[len];
	char cmd[128];

	sprintf(cmd, IS_UPGRADE_TOOL" -p -V %s", filename);
	if(silc_mgmtd_if_exec_system_cmd(cmd, buf, &len, 100000, silc_false) != 0)
	{
		unlink(filename);
		silc_cli_err_cmd_set_err_info("Failed to check upgrade package");
		return -1;
	}
	silc_cli_print(buf);
	if(strstr(buf, "Error:") != NULL)
	{
		unlink(filename);
		silc_cli_err_cmd_set_err_info("Invalid upgrade package");
		return -1;
	}
	return 0;
}

#define IS_CLI_SW_VERSOIN_FILE		"software_version.txt"
#define IS_CLI_UBOOT_VERSOIN_FILE	"uboot_version.txt"
#define IS_CLI_DTB_VERSOIN_FILE		"dtb_version.txt"
#define IS_CLI_FMAN_VERSOIN_FILE	"fman_version.txt"
#define IS_CLI_RCW_VERSOIN_FILE		"rcw_version.txt"

silc_cstr is_cli_cmd_get_verion(silc_cstr key, silc_cstr_array* p_arr)
{
	int loop;
	silc_cstr line;

	for(loop=0; loop<p_arr->length; loop++)
	{
		line = silc_cstr_array_get_quick(p_arr, loop);
		if(strncmp(line, key, strlen(key)) == 0)
		{
			return line+strlen(key);
		}
	}
	//silc_cli_err_cmd_set_err_info("Can not get %s version", key);
	return NULL;
}

int is_cli_cmd_print_upgrade_info(silc_cstr title, silc_cstr key, silc_cstr_array* p_arr, silc_cstr file)
{
	int len = 100;
	char buf[len];
	char cmd[256];
	silc_cstr ver;

	if (access(file, R_OK))
	{
		silc_cli_err_cmd_set_err_info("Can not access version file %s", file);
		return -1;
	}
	sprintf(cmd, "cat %s/%s|sed \"s/##/\\n/g\"|head -n 2|tail -n 1",
			IS_UPGRADE_PATH, file);
	if(silc_mgmtd_if_exec_system_cmd(cmd, buf, &len, 1000, silc_false) != 0)
	{
		silc_cli_err_cmd_set_err_info("Can not get the %s version file", file);
		return -1;
	}
	buf[strlen(buf)-1] = 0;
	if((ver=is_cli_cmd_get_verion(key, p_arr)) == NULL)
		return -1;
	silc_cli_print("%s: %s => %s\n", title, ver, buf);
	return 0;
}

void is_cli_cmd_upgrade_finish()
{
	char ch;
	// reboot
	silc_cli_print("Upgrade is done, reboot for immediately effect? (y|n):");
	ch = getchar();
	while(getchar() != '\n');
	if(ch == 'y')
	{
#ifndef SILC_MGMTD_LOCAL_DEBUG
		system("reboot");
#endif
	}
}

#define IS_UPGRADE_ITEM_NUM	5
int is_cli_cmd_upgrade_software(silc_cstr filename, silc_bool b_manufacture_upgrade)
{
	char ch;
	int len = 1000;
	char buf[len];
	silc_cstr_array *p_arr, *p_arr_upd;
	int flag = 0, loop_map;
	char* key_map[IS_UPGRADE_ITEM_NUM][2] = {
			{"software: ", "Software current version: "},
			{"uboot: ", "uboot version: "},
			{"fman: ", "fman version: "},
			{"rcw: ", "rcw version: "},
			{"vendor: ", "Vendor: "},
	};
	int warn_flag[IS_UPGRADE_ITEM_NUM] = {0, 1, 1, 1, 0};
	silc_cstr cur_ver, upd_ver;

	silc_cli_print("Checking the upgrade package...\n");
	if(is_cli_cmd_verify_software(filename) != 0)
		return -1;
	silc_cli_print(" Check OK.\n");

	if(silc_mgmtd_if_exec_system_cmd(IS_UPGRADE_TOOL" -S", buf, &len, 10000, silc_false) != 0)
	{
		silc_cli_err_cmd_set_err_info("Can not get the system version");
		return -1;
	}
	p_arr = silc_cstr_split(buf, "\n");
	if(!p_arr)
	{
		silc_cli_err_cmd_set_err_info("Can not parser the system version info");
		return -1;
	}

	if(silc_mgmtd_if_exec_system_cmd(IS_UPGRADE_TOOL" -U", buf, &len, 100000, silc_false) != 0)
	{
		silc_cli_err_cmd_set_err_info("Can not get the system upgrade version");
		return -1;
	}
	p_arr_upd = silc_cstr_split(buf, "\n");
	if(!p_arr)
	{
		silc_cli_err_cmd_set_err_info("Can not parser the system upgrade version info");
		return -1;
	}

    silc_cli_print("\nThe software upgrade list is:\n");
#if 0
    if(is_cli_cmd_print_upgrade_info("Software", "Software current version: ", p_arr, IS_CLI_SW_VERSOIN_FILE) != 0)
    	return -1;
    if (0 == access(IS_CLI_UBOOT_VERSOIN_FILE, R_OK))
    {
    	flag = 1;
    	is_cli_cmd_print_upgrade_info("Uboot", "uboot version: ", p_arr, IS_CLI_UBOOT_VERSOIN_FILE);
    }
    if (0 == access(IS_CLI_DTB_VERSOIN_FILE, R_OK))
    {
    	flag = 1;
   		is_cli_cmd_print_upgrade_info("DTB", "dtb version: ", p_arr, IS_CLI_DTB_VERSOIN_FILE);
    }
    if (0 == access(IS_CLI_FMAN_VERSOIN_FILE, R_OK))
    {
    	flag = 1;
    	is_cli_cmd_print_upgrade_info("FMAN", "fman version: ", p_arr, IS_CLI_FMAN_VERSOIN_FILE);
    }
    if (0 == access(IS_CLI_RCW_VERSOIN_FILE, R_OK))
    {
    	flag = 1;
    	is_cli_cmd_print_upgrade_info("RCW", "rcw version: ", p_arr, IS_CLI_RCW_VERSOIN_FILE);
    }
#else
    for(loop_map=0; loop_map<IS_UPGRADE_ITEM_NUM; loop_map++)
	{
		upd_ver = is_cli_cmd_get_verion(key_map[loop_map][0], p_arr_upd);
		if(upd_ver)
		{
			cur_ver = is_cli_cmd_get_verion(key_map[loop_map][1], p_arr);
			if(!cur_ver)
			{
				cur_ver = "NULL";
			}
			silc_cli_print("%s %s => %s\n", key_map[loop_map][0], cur_ver, upd_ver);
			if(warn_flag[loop_map])
				flag = 1;
		}
	}
    silc_cstr_array_destroy(p_arr);
    silc_cstr_array_destroy(p_arr_upd);
#endif
    if (flag)
    	silc_cli_print("WARNING: Upgrading Uboot/FMAN/RCW can not be interrupted, else the system will be destroyed!!!\n");
SELECT:
	silc_cli_print("The upgrade might take about 3 minutes to complete. Do you want to continue? (y|n):");
	ch = getchar();
	while(getchar() != '\n');
	if(ch == 'y')
	{
#ifndef SILC_MGMTD_LOCAL_DEBUG
		// run the upgrade in background so it won't exit when the shell exits
		if(!b_manufacture_upgrade)
		{
			if(silc_mgmtd_if_system("setsid "IS_UPGRADE_TOOL" -p -u > "IS_UPGRADE_LOG" & echo $! > "IS_UPGRADE_PIDFILE) != 0)
			{
				silc_cli_err_cmd_set_err_info("upgrade software failed");
				return -1;
			}
		}
		else
		{
			if(silc_mgmtd_if_system("setsid "IS_UPGRADE_TOOL" -p -M > "IS_UPGRADE_LOG" & echo $! >"IS_UPGRADE_PIDFILE) != 0)
			{
				silc_cli_err_cmd_set_err_info("manufacture upgrade failed");
				return -1;
			}
		}
		// show upgrade realtime progress
		silc_mgmtd_if_system(IS_UPGRADE_TOOL" -r");
		// check whether the upgrade is complete
		if(silc_mgmtd_if_system(IS_UPGRADE_TOOL" -a") == 2)
			silc_cli_print("Please reload the device to complete the upgrade.\n");
#endif
		return 0;
	}
	else if(ch == 'n')
	{
		return 1; // was 0;
	}
	else
		goto SELECT;

	is_cli_cmd_upgrade_finish();

	return 0;
}


int is_cli_cmd_get_image(silc_list* p_token_list, silc_bool *b_manufacture_upgrade)
{
	silc_cli_token *p_token;
	silc_bool b_manufacture_vendor = silc_false;
	silc_cstr user = NULL, passwd = NULL, url = NULL, image=NULL;

	if(silc_cli_get_product_info()->get_manufacture_mode_func)
	{
		b_manufacture_vendor = silc_cli_get_product_info()->get_manufacture_mode_func();
	}

	/* Token sequence: upgrade [manufacture] file-url XXXX */
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "file-url") == 0)
			url = p_token->val_str;
		else if(strcmp(p_token->name, "manufacture") == 0)
			*b_manufacture_upgrade = silc_true;
		else if(strcmp(p_token->name, "ftp-user") == 0)
			user = p_token->val_str;
		else if(strcmp(p_token->name, "ftp-password") == 0)
			passwd = p_token->val_str;
		else if(strcmp(p_token->name, "image") == 0)
			image = p_token->val_str;
	}

	if( (b_manufacture_vendor && !*b_manufacture_upgrade) || // 1. manufacture vendor: standard upgrade forbidden;
	   (!b_manufacture_vendor &&  *b_manufacture_upgrade) )  // 2. normal vendor: manufacture upgrade forbidden;
		return -1;

	/* These two cases are allowed:
	 * ( b_manufacture_vendor &&  *b_manufacture_upgrade)
	 * (!b_manufacture_vendor && !*b_manufacture_upgrade)
	 */

	if(image)
	{
		char path[256], cmd[512];
		sprintf(path, "%s/%s", LOCAL_IMAGE_PATH, image);
		if(access(path, F_OK))
		{
			silc_cli_print("Image %s not found.\n", path);
			return -1;
		}
		sprintf(cmd, "cp -f %s %s", path, IS_UPGRADE_IMG);
		if(silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 1000000, silc_false) != 0)
		{
			silc_cli_print("Copy image %s error.\n", path);
			return -1;
		}
		return 0;
	}

	if(silc_cli_upload_file(url, user, passwd, IS_UPGRADE_IMG) != 0)
		return -1;
	silc_cli_print("Upload is done.\n");
	return 0;
}


int manufacture_reset_system()
{
	char err_info[200];

	silc_cli_print("System is reset\n");
	if(silc_cli_cmd_do_simple_action("/action/system/reset", "", err_info, 200) != 0)
	{
		silc_cli_err_cmd_set_err_info("failed to reset the system");
		return -1;
	}
	return 0;
}

int is_cli_cmd_upgrade_do(silc_list* p_token_list)
{
	silc_bool b_manufacture_upgrade = silc_false;
	int ret = -1;

	if(silc_cli_cmd_do_simple_action("/action/system/check-is-admin", "true", NULL, 0) != 0)
	{
		silc_cli_err_cmd_set_err_info("no privilege to upgrade");
		return -1;
	}
	ret = silc_mgmtd_if_system(IS_UPGRADE_TOOL" -a");
	if(ret == 1)
	{
		silc_cli_print("Another upgraded is in progress, please don't power off the device.\n");
		silc_mgmtd_if_system(IS_UPGRADE_TOOL" -r");
		return 0;
	}
	else if(ret == 2)
	{
		silc_cli_print("The upgrade is done, please reload the device to complete the upgrade.\n");
		return 0;
	}

	if (0 == access(IS_UPGRADE_IMG, F_OK))
	{
		if (0 != access(IS_UPGRADE_IMG, W_OK))
		{
			silc_cli_print("An image is being uploaded, please check again later.\n");
			return 0;
		}
		else
		{
			char ch;
			silc_cli_print("An image is already uploaded, do you want to use it for upgrade? (y|n):");
			ch = getchar();
			while(getchar() != '\n');
			if (ch == 'y')
			{
				if ((ret=is_cli_cmd_upgrade_software(IS_UPGRADE_IMG, b_manufacture_upgrade)) == -1)
					return -1;

				if(ret == 0 && b_manufacture_upgrade)
				{
					if (manufacture_reset_system() != 0)
						return -1;
				}
				return 0;
			}
		}
	}

	if (is_cli_cmd_get_image(p_token_list, &b_manufacture_upgrade) != 0)
		return -1;

	if ((ret=is_cli_cmd_upgrade_software(IS_UPGRADE_IMG, b_manufacture_upgrade)) == -1)
		return -1;

	/* ret: upgrade done on 0, or skip on 1 */
	if (ret == 0 && b_manufacture_upgrade)
	{
		if (manufacture_reset_system() != 0)
			return -1;
	}

	return 0;
}

