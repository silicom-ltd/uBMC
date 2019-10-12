/*
 *
 */
#include "silc_common.h"
#include "silc_mgmtd_lib_err.h"
#include "silc_mgmtd_error.h"

#define SILC_MGMTD_ERR_INFO_MAX_LEN		1024
static silc_mgmtd_err s_silc_mgmtd_current_err;
static char s_silc_mgmtd_current_err_info[SILC_MGMTD_ERR_INFO_MAX_LEN] = "";

static silc_cstr s_silc_mgmtd_err_cstr[] = {
		"OK",
		"Operation not supported",
		"C library Error",
		"Mgmtd Library Error",
		"Mgmtd Callback Error",

		"Memory database node does not exist",
		"Memory database node already exist",
		"Memory database node is locked",
		"Callback function error is not defined",
		"Traversal the memory database node tree caused some error",
		"Memory database node callback function is timeout",
		"Memory database node is not a configuration node",
		"Memory database node's parent has no child",
		"Memory database node is not dynamic",
		"Memory database node's parent is not exist"

		"Configuration file is locked",
		"Invalid node string in configuration file",
		"Can not move configuration file",

		"Invalid operation for the node",
		"Operation is timeout",
		"Delete with reference node",

		"Unknown error"
};


silc_cstr silc_mgmtd_err_to_str(silc_mgmtd_err err)
{
	if(err >= MGMTD_ERR_MAX)
		return "Unknown error!";

	if(err == MGMTD_ERR_CLIB)
		return strerror(errno);

	if(err == MGMTD_ERR_MGMTD_LIB)
		return silc_mgmtd_lib_err_str();

	if(err == MGMTD_ERR_CALLBACK)
		return "Bug!!Callback error here!";

	if(strlen(s_silc_mgmtd_current_err_info))
	{
		char info[SILC_MGMTD_ERR_INFO_MAX_LEN];
		sprintf(info, "%s %s", s_silc_mgmtd_err_cstr[err], s_silc_mgmtd_current_err_info);
		strcpy(s_silc_mgmtd_current_err_info, info);
		return s_silc_mgmtd_current_err_info;
	}
	return s_silc_mgmtd_err_cstr[err];
}

silc_cstr silc_mgmtd_err_str()
{
	return silc_mgmtd_err_to_str(s_silc_mgmtd_current_err);
}

void silc_mgmtd_err_set(silc_mgmtd_err err, const char* info)
{
	s_silc_mgmtd_current_err = err;
	if(info)
	{
		strncpy(s_silc_mgmtd_current_err_info, info, SILC_MGMTD_ERR_INFO_MAX_LEN);
		s_silc_mgmtd_current_err_info[SILC_MGMTD_ERR_INFO_MAX_LEN-1] = 0;
	}
	else
		s_silc_mgmtd_current_err_info[0] = 0;
}

silc_mgmtd_err silc_mgmtd_err_get()
{
	return s_silc_mgmtd_current_err;
}
