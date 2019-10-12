/* this file is created by silc_mgmtd_inst_func_header_gen.py */ 

#include "silc_mgmtd_memdb.h"
#include "is_mgmtd_func_def.h"


/* define memdb node err enum */ 
enum
{
    IS_MGMTD_ERR_BMC_NULL = 2000,
    IS_MGMTD_ERR_BMC_CDROM_ISO_FILE_NOT_FOUND,
    IS_MGMTD_ERR_BMC_CDROM_ISO_FILE_IN_USE,
    IS_MGMTD_ERR_BMC_CDROM_USERNAME_OR_PASSWD_ERR,
    IS_MGMTD_ERR_BMC_CDROM_REMOTE_HOST_UNAVAILABLE,
    IS_MGMTD_ERR_BMC_CDROM_ATTACHED,
    IS_MGMTD_ERR_BMC_CDROM_NOT_ATTACHED,
    IS_MGMTD_ERR_BMC_MAX
};

extern int is_mgmtd_bmc_upgrade(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_bmc_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_bmc_status(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_bmc_notify(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_bmc_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 



/* declare node reference callback functions */ 


