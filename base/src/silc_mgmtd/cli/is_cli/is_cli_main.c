/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_types.h"
#include "silc_cli_terminal.h"
#include "silc_cli_readline.h"
#include "silc_cli_common.h"
#include "is_cli_func.h"

#define EULA_FILE	"/etc/EULA.txt"
#define EULA_STATUS	"/config/EULA.accept"

static void accept_eula()
{
	char buf[128];
	if(access(EULA_FILE, F_OK) != 0 || access(EULA_STATUS, F_OK) == 0)
		return;
	printf("******** License Agreement ********\n"
			"You must accept the license agreement before continuing.\n"
			"Please press ENTER to view the agreement and press Q when you finish reading.\n");
	fgets(buf, 128, stdin);
	system("less "EULA_FILE);
	do
	{
		printf("\nWill you accept the agreement? [y|n]");
		fgets(buf, 128, stdin);
	} while (strcasecmp(buf, "y\n") != 0 && strcasecmp(buf, "n\n") != 0);
	if(strcasecmp(buf, "n\n") == 0)
	{
		printf("Sorry, you must accept the license agreement before continuing.\n");
		exit(0);
	}
	system("sudo touch "EULA_STATUS);
	return;
}

#define WARNING_FILE	"/tmp/is_global_warning"

static void show_warning()
{
	if(access(WARNING_FILE, F_OK) != 0)
		return;
	printf("WARNING!!\n");
	system("cat "WARNING_FILE);
}

#define IS_UPLOAD_DIR	"/var/images"

static int cli_process_c_params(int argc, const char** argv)
{
	silc_cstr cmd, bin, opt, dir;
	silc_cstr_array* arg_list;
	char path[256];

	cmd = (silc_cstr)argv[2];

	arg_list = silc_cstr_split(cmd, " ");
	if(!arg_list)
	{
		SILC_ERR("[%s] silc_cstr_split error", __func__);
		return -1;
	}
	if(arg_list->length < 3)
	{
		SILC_ERR("Unsupported cmd: %s", cmd);
		return -1;
	}

	bin = silc_cstr_array_get_quick(arg_list, 0);
	if(strcmp(bin, "scp"))
	{
		SILC_ERR("Unsupported cmd: %s", cmd);
		return -1;
	}

	opt = silc_cstr_array_get_quick(arg_list, 1);
	if(strcmp(opt, "-t") && strcmp(opt, "-f") && strcmp(opt, "-d"))
	{
		// -r(recursive) is not allowed
		SILC_ERR("Unsupported cmd: %s", cmd);
		return -1;
	}

	dir = silc_cstr_array_get_quick(arg_list, arg_list->length-1);
	if(NULL == realpath(dir, path))
	{
		// creating a new file in dir is not allowed
		SILC_ERR("Not found path cmd: %s", cmd);
		return -1;
	}
	if(access(path, R_OK) != 0)
	{
		SILC_ERR("Not accessible path cmd: %s", cmd);
		return -1;
	}
	if(strncmp(path, IS_UPLOAD_DIR, strlen(IS_UPLOAD_DIR)))
	{
		SILC_ERR("Not allowed path cmd: %s", cmd);
		return -1;
	}
	SILC_LOG("SCP cmd: %s", cmd);

	system(cmd);
	return 0;
}

int main(int argc, const char** argv)
{
    silc_cstr line;
    silc_list token_list;
    silc_cli_cmd* p_cmd;

    if(argc > 1)
    {
    	// this will be called when scp
    	if(strcmp(argv[1], "-c") == 0 && argc > 2)
    	{
    		return cli_process_c_params(argc, argv);
    	}
    	else
    	{
    		SILC_ERR("Unknown cli arg: %s, argc: %d", argv[1], argc);
    		return -1;
    	}
    }

    accept_eula();

    show_warning();

    silc_time_lib_init();

    silc_cli_rl_init(argv[0]);

    if(is_cli_init() < 0)
    {
        SILC_ERR("Initialize is_cli failed!\n");
        return -1;
    }

    if(silc_mgmtd_if_client_init("127.0.0.1", 2626, -1, 0) < 0)
    {
        SILC_ERR("Connect to mgmtd failed!\n");
        return -1;
    }

    if(silc_cli_set_connect_login_info() != 0)
    {
        SILC_ERR("Set session login info failed!\n");
        return -1;
    }

    if(silc_cli_session_timeout_init() != 0)
    {
        SILC_ERR("Set session expire timeout failed!\n");
        return -1;
    }

    silc_cli_mode_set_dev_name();

    silc_cli_mode_curr_set("user_exec");

    while((line = silc_cli_rl_readline()))
    {
        p_cmd = silc_cli_rl_parser_cmd(line, &token_list);
        if(!p_cmd)
            goto CONTINUE;

        silc_cli_session_timeout_update();
        if(silc_cli_do_cmd(p_cmd, &token_list) != 0)
            silc_cli_print("%% Command failed!\n");
        else
            silc_cli_print("\n");

CONTINUE:
        silc_cli_session_timeout_update();
        silc_cli_rl_free_token_list(&token_list);
        silc_cli_rl_free_readline(line);
    }
    return 0;
}

