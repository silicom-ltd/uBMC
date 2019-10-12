/*
 *
 */
#include "silc_common.h"
#include "silc_mgmtd_lib_err.h"
#include "silc_cli_error.h"

static silc_cli_errno s_silc_cli_curr_errno;
char g_silc_cli_cmd_err_info[1024];

static silc_cstr s_silc_cli_err_cstr[] = {
		"OK",
		"C library Error",
		"CLI Library Error",
		"CLI Callback Error",

		"Unknown command word",
		"Unknown return error",
		"No command for this line",
		"Unfinished parameter",

		"Unknown error"
};


silc_cstr silc_cli_err_to_str(silc_cli_errno err)
{
	if(err >= CLI_ERR_MAX)
		return "Unknown error!";

	if(err == CLI_ERR_SYSTEM)
		return strerror(errno);

	if(err == CLI_ERR_LIB)
		return silc_mgmtd_lib_err_str();

	if(err == CLI_ERR_CALLBACK)
		return "Bug!!Callback error here!";

	return s_silc_cli_err_cstr[err];
}

silc_cstr silc_cli_err_str()
{
	return silc_cli_err_to_str(s_silc_cli_curr_errno);
}

void silc_cli_err_set(silc_cli_errno err)
{
	s_silc_cli_curr_errno = err;
}

