/* this file is created by silc_mgmtd_inst_func_header_gen.py */ 

#include "silc_mgmtd_memdb.h"


/* define memdb node err enum */ 
enum
{
    IS_MGMTD_ERR_NULL = 0,
    IS_MGMTD_ERR_BASE_NOT_SUPPORT,
    IS_MGMTD_ERR_BASE_FORBIDDEN,
    IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR,
    IS_MGMTD_ERR_BASE_NODE_NOT_EXIST,
    IS_MGMTD_ERR_BASE_MISS_PARAM,
    IS_MGMTD_ERR_BASE_INVALID_PARAM,
    IS_MGMTD_ERR_BASE_EXEC_FAILED,
    IS_MGMTD_ERR_BASE_GET_INFO_FAILED,
    IS_MGMTD_ERR_BASE_CREATE_NOTIFY,
    IS_MGMTD_ERR_BASE_SEND_NOTIFY,
    IS_MGMTD_ERR_BASE_ADD_RSP_NODE,
    IS_MGMTD_ERR_BASE_CSTR_SPLIT_FAILED,
    IS_MGMTD_ERR_BASE_MALLOC_FAILED,
    IS_MGMTD_ERR_BASE_NOT_ENOUGH_BUF,
    IS_MGMTD_ERR_SYSTEM_CERT_KEY_IN_USE,
    IS_MGMTD_ERR_SYSTEM_CERT_KEY_MISMATCH,
    IS_MGMTD_ERR_SYSTEM_DISK_FULL,
    IS_MGMTD_ERR_SYSTEM_IP_MASK_MISMATCH,
    IS_MGMTD_ERR_SYSTEM_TERMINAL_TYPE_INVALID,
    IS_MGMTD_ERR_SYSTEM_COM_SPEED_INVALID,
    IS_MGMTD_ERR_SYSTEM_SET_COM_FAILED,
    IS_MGMTD_ERR_AAA_USER_NAME_LEN,
    IS_MGMTD_ERR_AAA_USER_NAME_CHAR,
    IS_MGMTD_ERR_AAA_USER_FORBIDDEN,
    IS_MGMTD_ERR_AAA_USER_NO_PRIVIL,
    IS_MGMTD_ERR_AAA_PASSWD_LEN_6_40,
    IS_MGMTD_ERR_AAA_PASSWD_LEN_7_40,
    IS_MGMTD_ERR_AAA_PASSWD_TOO_WEAK,
    IS_MGMTD_ERR_AAA_PRIVIL_INVALID,
    IS_MGMTD_ERR_AAA_PRIVIL_FORBIDDEN,
    IS_MGMTD_ERR_AAA_MAPPED_USER_NOTALLOWED,
    IS_MGMTD_ERR_AAA_MAPPED_USER_INUSE,
    IS_MGMTD_ERR_AAA_RADIUS_TAC_CONFLICT,
    IS_MGMTD_ERR_AAA_AUTH_FAIL,
    IS_MGMTD_ERR_AAA_SERVER_DUPLICATED,
    IS_MGMTD_ERR_SNMP_PASSWD_INVALID,
    IS_MGMTD_ERR_SNMP_AUTH_INVALID,
    IS_MGMTD_ERR_SNMP_COMMUNITY_NAME_INVALID,
    IS_MGMTD_ERR_SNMP_USER_NAME_INVALID,
    IS_MGMTD_ERR_MAX
};

/* declare memdb init functions */ 
extern int is_mgmtd_memdb_init();

/* declare node callback functions */ 
extern int is_mgmtd_system_upgrade(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_system_mgmt_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_system_service_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_system_misc_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_system_status(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_system_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_radius_upgrade(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_radius_static_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_radius_server_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_tacacs_upgrade(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_tacacs_static_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_tacacs_server_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_unix_upgrade(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_unix_static_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_unix_user_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_aaa_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_snmp_upgrade(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_snmp_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_threshold_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_snmp_status(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

extern int is_mgmtd_snmp_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry); 

/* declare node reference callback functions */ 


