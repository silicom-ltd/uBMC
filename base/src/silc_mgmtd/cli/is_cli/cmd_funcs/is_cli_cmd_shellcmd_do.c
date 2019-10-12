/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
struct command_map
{
	const char *cmd_str;
	const char *cmd_path;
};

struct command_map cmd_map[]={
							 {"dhclient","/sbin/dhclient"},
							 {"dmesg","/bin/dmesg"},
							 {"ethtool","/usr/sbin/ethtool"},
							 {"ip","/sbin/ip"},
//							 {"iptables","/usr/sbin/iptables"},
							 {"ifconfig","/sbin/ifconfig"},
							 {"openssl","/usr/bin/openssl"},
							 {"ping","/bin/ping"},
							 {"traceroute","sudo /usr/bin/traceroute"},
							 {"vconfig","/sbin/vconfig"},
							 {NULL,NULL}
							};

const char* shcmd_find_cmd_in_map(struct command_map*map,char* cmd)
{
	int i = 0;
	if(cmd == NULL)
		return NULL;
	while(map[i].cmd_str != NULL)
	{
		if(strcmp(map[i].cmd_str, cmd) == 0)
		{
			return map[i].cmd_path;
		}
		i++;
	}
	return NULL;
}

static int shcmd_exec(const char* cmd_path, char* cmd)
{
	char**  argv = NULL;
	int ret = -1, i = 0;

	silc_cstr_array *p_arg_arr = silc_cstr_split(cmd, " ");
	if(!p_arg_arr)
	{
		silc_cli_print("Fail to split commands\n");
		goto OUT;
	}
	argv = malloc(sizeof(char*)*(p_arg_arr->length+1));
	if(!argv)
	{
		silc_cli_print("Fail to malloc argv\n");
		goto OUT;
	}
	for(i=0; i<p_arg_arr->length; i++)
	{
		argv[i] = silc_cstr_array_get_quick(p_arg_arr, i);
	}
	argv[i] = NULL;
	ret = execvp(cmd_path,argv);
OUT:
	if(p_arg_arr)
		silc_cstr_array_destroy(p_arg_arr);
	if(argv)
		free(argv);
	return ret;
}

int shcmd_system(const char* cmd_path, char* cmd)
{
    int status;
    pid_t pid;
    struct sigaction sa;
    struct sigaction intr, quit;
    sigset_t omask;

    if (NULL == cmd) {
        return 0;
    }
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, &intr);
    sigaction(SIGQUIT, &sa, &quit);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sa.sa_mask, &omask);
    pid = fork();
    if (pid == (pid_t)0)
    {
        sigaction(SIGINT, &intr, (struct sigaction *)NULL);
        sigaction(SIGQUIT, &quit, (struct sigaction *)NULL);
        sigprocmask(SIG_SETMASK, &omask, (sigset_t *)NULL);
        /* Exec the shell.  */
        shcmd_exec(cmd_path, cmd);
        _exit(127);
    }
    else if (pid < (pid_t) 0)
    {
        /* fork() error -1  */
        status = -1;
    }
    else
    {
        /* Note the system() is a cancellation point.  But since we call
         waitpid() which itself is a cancellation point we do not have to do anything here. */
        if (waitpid(pid, &status, 0) != pid)
        {
            status = -1;
        }
    }
    sigaction(SIGINT, &intr, (struct sigaction *)NULL);
    sigaction(SIGQUIT, &quit, (struct sigaction *)NULL);
    sigprocmask(SIG_SETMASK, &omask, (sigset_t *)NULL);
    return status;
}

int is_cli_cmd_shellcmd_do(silc_list* p_token_list)
{
	silc_cli_token *p_token,*p_l1_token;
	char cmd[256] = {0};
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "shellcmd") == 0)
		{
			p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
			if(p_l1_token)
			{
				const char* cmd_path = shcmd_find_cmd_in_map(cmd_map,p_l1_token->name);
				if(cmd_path)
				{
					if(strlen(p_l1_token->val_str) > 224)
					{
						silc_cli_print("Command arguments too long\n");
						break;
					}
					if(strncmp(cmd_path, "sudo", strlen("sudo")) == 0)
					{
						sprintf(cmd,"%s %s",cmd_path,p_l1_token->val_str);
						cmd_path = "/usr/bin/sudo";
					}
					else
						sprintf(cmd,"%s %s",p_l1_token->name,p_l1_token->val_str);
					shcmd_system(cmd_path, cmd);
					return 0;
				}
			}
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

