
#ifndef SILC_CLI_ERROR_H_
#define SILC_CLI_ERROR_H_

#include "silc_common.h"
#include "silc_mgmtd_lib_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	CLI_ERR_NULL = 0,
	CLI_ERR_SYSTEM,
	CLI_ERR_LIB,
	CLI_ERR_CALLBACK,

	CLI_ERR_TYPES_TOKEN_NOTEXIST,
	CLI_ERR_TYPES_ERR_NOTFIND,

	CLI_ERR_READLINE_NOCMD,
	CLI_ERR_READLINE_NOTOKENVAL,

	CLI_ERR_MAX,
}silc_cli_errno;

silc_cstr silc_cli_err_str(void);

void silc_cli_err_set(silc_cli_errno err);

silc_cli_errno silc_cli_err_get();

extern char g_silc_cli_cmd_err_info[1024];

#define silc_cli_err_cmd_set_err_info(fmt, ...)		\
	do {\
		sprintf(g_silc_cli_cmd_err_info, fmt, ## __VA_ARGS__); \
	}while(0)

#define silc_cli_err_cmd_get_err_info()		(g_silc_cli_cmd_err_info)

#define silc_cli_err_cmd_set_incomplete_cmd(p_token)	\
	do { \
		char missed[200] = ""; \
		silc_cli_token* p_sub_token; \
		silc_list_for_each_entry(p_sub_token, &p_token->sub_token_list, node) {\
			strcat(missed, p_sub_token->name); \
			strcat(missed, "|"); \
		} \
		if(strlen(missed)) \
			missed[strlen(missed)-1] = 0;\
		silc_cli_err_cmd_set_err_info("Incomplete command, need %s", missed); \
	}while(0)

#define silc_cli_err_cmd_set_invalid_cmd()			silc_cli_err_cmd_set_err_info("invalid command")
#define silc_cli_err_cmd_set_invalid_param(param)	silc_cli_err_cmd_set_err_info("invalid parameter(%s)", param)
#define silc_cli_err_cmd_set_rsp_error(err_info)	silc_cli_err_cmd_set_err_info("remote return error, %s", err_info)
#define silc_cli_err_cmd_set_send_req_failed()		silc_cli_err_cmd_set_err_info("send request failed, %s", silc_mgmtd_lib_err_str());
#define silc_cli_err_cmd_set_create_req_failed()	silc_cli_err_cmd_set_err_info("create request failed, %s", silc_mgmtd_lib_err_str())
#define silc_cli_err_cmd_set_req_addnode_failed()	silc_cli_err_cmd_set_err_info("add request node failed, %s", silc_mgmtd_lib_err_str())
#define silc_cli_err_cmd_set_req_add_node_failed(node, val)	silc_cli_err_cmd_set_err_info("add request node(%s,%s) failed, %s", node, val, silc_mgmtd_lib_err_str())


#ifdef __cplusplus
}
#endif

#endif /* SILC_CLI_ERROR_H_ */
