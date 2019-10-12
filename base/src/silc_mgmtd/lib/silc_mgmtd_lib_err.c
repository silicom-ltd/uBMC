/*
 *
 */
#include "silc_common.h"

#include "../inc/silc_mgmtd_lib_err.h"

static silc_mgmtd_lib_err s_silc_mgmtd_lib_curr_err;
static int s_silc_mgmtd_sys_clib_errno;

static silc_cstr s_silc_mgmtd_lib_err_cstr[] = {
		"OK",
		"Out of Memory",
		"System C library Error",

		"Variables type name is invalid",
		"Variables type is unknown",
		"Value string is failed to be convert",
		"Value string buffer is not enough",
		"Value string is invalid",
		"Value is out of range",

		"Connection timed out",
		"Connection has been closed",
		"Try again for reading from connection",
		"Connection is not exist",
		"Connection ID is NULL",
		"Pop data from connection is failed",
		"Creating socket for connection is failed",
		"Connecting is timeout",
		"Connection send message is failed"

		"Node is not exist",
		"Node traversal callback function is failed",
		"Node path is not exist",
		"Request type is unknown",
		"Request buffer is not enough",
		"Message string is invalid",
		"Message sequence string is invalid",
		"Message head is not exist",
		"Message end is not exist",
		"Message item head is not exist",
		"Message is empty",
		"Message waiting is timeout",
		"Message has wrong sequence number",
		"Execute system command timeout",
		"Failed to set login info",
		"Failed to get the connection idle timeout",
		"Failed to set received notify path",
		"Notify path is not enough",

		"Unknown error"
};



silc_cstr silc_mgmtd_lib_err_by_no(silc_mgmtd_lib_err err)
{
	if(err >= LIB_ERR_MAX)
		return s_silc_mgmtd_lib_err_cstr[LIB_ERR_MAX];

	if(err == LIB_ERR_SYS_CLIB)
		return strerror(s_silc_mgmtd_sys_clib_errno);

	return s_silc_mgmtd_lib_err_cstr[err];
}

silc_cstr silc_mgmtd_lib_err_str(void)
{
	return silc_mgmtd_lib_err_by_no(s_silc_mgmtd_lib_curr_err);
}

void silc_mgmtd_lib_err_set(silc_mgmtd_lib_err err)
{
	s_silc_mgmtd_lib_curr_err = err;
	if(err == LIB_ERR_SYS_CLIB)
		s_silc_mgmtd_sys_clib_errno = errno;
}

