#include "silc_common.h"
#include "silc_mgmtd_interface.h"
#include "silc_cli_common.h"

int main(int argc, char * argv [])
{
	char err_info[200];

    silc_time_lib_init();

    if(silc_mgmtd_if_client_init("127.0.0.1", 2626, -1, 0) < 0)
    {
        printf("Connect to mgmtd failed!\n");
        return -1;
    }

    if(silc_cli_set_connect_login_info() != 0)
    {
    	printf("Set session login info failed!\n");
        return -1;
    }

    if(argc > 1)
    {
    	if(strcmp(argv[1], "reset-admin-passwd") == 0)
    	{
    		if(silc_cli_cmd_do_simple_action("/action/aaa/reset-admin-passwd", "", err_info, 200) != 0)
    		{
    			printf("Reset admin password failed! Error: %s.\n", err_info);
    			return -1;
    		}
    	}
    	else if(strcmp(argv[1], "reset-config") == 0)
    	{
    		if(silc_cli_cmd_do_simple_action("/action/system/load-default-config", "", err_info, 200) != 0)
    		{
    			printf("Reset system configuration failed! Error: %s.\n", err_info);
    			return -1;
    		}
    	}
    	else
    	{
    		printf("Unknown arg: %s, argc: %d", argv[1], argc);
    		return -1;
    	}
    }

    silc_mgmtd_if_client_shutdown();
    return 0;
}
