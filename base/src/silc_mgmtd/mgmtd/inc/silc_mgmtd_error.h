
#ifndef SILC_MGMTD_ERRNO_H_
#define SILC_MGMTD_ERRNO_H_

#include "silc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	MGMTD_ERR_NULL = 0,
	MGMTD_ERR_OP_NOT_SUPPORTED,
	MGMTD_ERR_CLIB,
	MGMTD_ERR_MGMTD_LIB,
	MGMTD_ERR_CALLBACK,

	MGMTD_ERR_MEMDB_NODE_NOTEXIST,
	MGMTD_ERR_MEMDB_NODE_EXIST,
	MGMTD_ERR_MEMDB_NODE_LOCKED,
	MGMTD_ERR_MEMDB_CB_ERR_NOTFIND,
	MGMTD_ERR_MEMDB_TRAVERSAL_ERR,
	MGMTD_ERR_MEMDB_CB_TIMEOUT,
	MGMTD_ERR_MEMDB_NOT_CONFIG_NODE,
	MGMTD_ERR_MEMDB_PARENT_NO_CHILD,
	MGMTD_ERR_MEMDB_NOT_DYNC_NODE,
	MGMTD_ERR_MEMDB_PARENT_NOTEXIST,

	MGMTD_ERR_CFG_LOCKED,
	MGMTD_ERR_CFG_INVALID_NODESTR,
	MGMTD_ERR_CFG_MV_FILE_FAILED,


	MGMTD_ERR_OP_INVALID_AUTHORITY,
	MGMTD_ERR_OP_TIMEOUT,
	MGMTD_ERR_OP_REFERENCE,

	MGMTD_ERR_VNODE_INVALID_PATH,

	MGMTD_ERR_MAX,
}silc_mgmtd_err;

silc_cstr silc_mgmtd_err_str(void);

void silc_mgmtd_err_set(silc_mgmtd_err err, const char* info);

silc_mgmtd_err silc_mgmtd_err_get();


#define SILC_MGMTD_ERR(err)	\
		do { silc_mgmtd_err_set(err, NULL); goto ERR_RET;} while(0)

#define SILC_MGMTD_ERR_INFO(err, info)	\
	do { silc_mgmtd_err_set(err, info); goto ERR_RET;} while(0)


#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_ERRNO_H_ */
