
#ifndef SILC_MGMTD_INTERFACE_H_
#define SILC_MGMTD_INTERFACE_H_

#include "silc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VENDOR_DEFAULT	0

typedef uint32_t silc_mgmtd_vendor_id;

#define SILC_MGMTD_PRODUCT_ID g_silc_mgmtd_product_id
#define SILC_MGMTD_PRODUCT_NAME g_silc_mgmtd_product_name

extern silc_cstr g_silc_mgmtd_product_name;
extern uint32_t g_silc_mgmtd_product_id;

typedef enum
{
	OP_STATE_NULL = 0,
	OP_STATE_RUNNING,
	OP_STATE_FINISHED,
	OP_STATE_TIMEOUT,
	OP_STATE_MAX,
}silc_mgmtd_op_state;

/*
 * --- NODE ---
 * Node is the smallest element of recording unit
 * Each node has one parent and some children.
 */
typedef struct silc_mgmtd_if_node_s
{
	silc_list_node node;
	silc_cstr	 name;				// node name
	silc_cstr  val_str;			// node value string
	silc_bool  val_str_dync;		// node value string is dynamic malloc
	silc_bool  add_dync;
	silc_bool  has_child;			//used for stats query, for callbacks to indicate whether a node may have child
	silc_list	 sub_node_list;		// sub node list
}silc_mgmtd_if_node;

typedef enum
{
	TRAP_EVENT_NORMAL = 0,
	TRAP_EVENT_WARNING = 1,
	TRAP_EVENT_FAULT = 2,
	TRAP_EVENT_RECOVER = 3,
	TRAP_EVENT_UP = 4,
	TRAP_EVENT_DOWN = 5,
	TRAP_EVENT_ON = 6,
	TRAP_EVENT_OFF = 7,
	TRAP_EVENT_CONNECT = 8,
	TRAP_EVENT_DISCONNECT = 9,
}silc_mgmtd_trap_event;

typedef enum
{
	SILC_MGMTD_IF_LEVEL_NULL = 0,
	SILC_MGMTD_IF_LEVEL_READONLY,
	SILC_MGMTD_IF_LEVEL_NORMAL,
	SILC_MGMTD_IF_LEVEL_ADMIN,
	SILC_MGMTD_IF_LEVEL_MAX,
}silc_mgmtd_if_privilege;

typedef enum
{
	SILC_MGMTD_IF_CLIENT_NULL = 0,
	SILC_MGMTD_IF_CLIENT_SNMP,
	SILC_MGMTD_IF_CLIENT_SWITCH,
	SILC_MGMTD_IF_CLIENT_LOCAL,	// for CLI
	SILC_MGMTD_IF_CLIENT_SSH,	// for CLI
	SILC_MGMTD_IF_CLIENT_TELNET,// for CLI
	SILC_MGMTD_IF_CLIENT_WEB,
	SILC_MGMTD_IF_CLIENT_NETCONF,
	SILC_MGMTD_IF_CLIENT_CONSOLE,
	SILC_MGMTD_IF_CLIENT_MAX,
}silc_mgmtd_if_client_type;

typedef enum
{
	SILC_MGMTD_IF_REQ_NULL = 0,
	SILC_MGMTD_IF_REQ_ADD,			// add dynamic config node
	SILC_MGMTD_IF_REQ_MODIFY,		// modify config node
	SILC_MGMTD_IF_REQ_DELETE,		// delete dynamic config node
	SILC_MGMTD_IF_REQ_QUERY,		// query the node itself
	SILC_MGMTD_IF_REQ_QUERY_CHILD, 	// query the son and the daughter
	SILC_MGMTD_IF_REQ_QUERY_SUB,	// query all the offspring.
	SILC_MGMTD_IF_REQ_QUERY_WILD,
	SILC_MGMTD_IF_REQ_ACTION,
	SILC_MGMTD_IF_REQ_NOTIFY,
	SILC_MGMTD_IF_REQ_CHECK_ADD,		// confirm the request input parameters
	SILC_MGMTD_IF_REQ_CHECK_MODIFY,		// confirm the request input parameters
	SILC_MGMTD_IF_REQ_CHECK_DELETE,		// confirm the request input parameters
	SILC_MGMTD_IF_REQ_UPGRADE,
	SILC_MGMTD_IF_REQ_MAX,
}silc_mgmtd_if_req_type;

#define SILC_MGMTD_QUERY_MAX_LEVEL		16
#define SILC_MGMTD_IF_NAME_MAX_LEN		64
#define SILC_MGMTD_IF_PATH_MAX_LEN		1024
#define SILC_MGMTD_IF_BUFFER_MAX_LEN	(1024*512)

typedef struct silc_mgmtd_conn_s silc_mgmtd_conn;
typedef struct silc_mgmtd_if_item_s
{
	silc_list_node node;
	silc_mgmtd_if_req_type type;
	silc_cstr path_str;	// entry root path of item
	silc_cstr return_str;
	silc_mgmtd_conn* conn_entry;
	silc_mgmtd_if_node* p_node_root;
}silc_mgmtd_if_item;


typedef struct silc_mgmtd_conn_msg_s
{
	silc_list_node		node;
	silc_mgmtd_conn*	conn_entry;
	char*				data;
	//below are generate from data;
	uint32_t	seq_num;
	uint64_t	timeout_abs_ms;
	silc_list		if_recv_items;
	silc_list		if_send_items;
	int			ret;
	silc_mgmtd_op_state state;
}silc_mgmtd_msg;

#if 0
#define silc_mgmtd_req_dbg(p_req, msg, fmt, ...)	\
	fprintf(stderr, "[%5u] state %d %10s "fmt"\n", (p_req)->state, (p_req)->seq_num, msg, ## __VA_ARGS__)
#else
#define silc_mgmtd_req_dbg(p_req, msg, fmt, ...)	do {} while(0)
#endif
/*
 * --- REQUEST ---
 * One item can execute add/modify/delete/query command.
 * Only config node can be add/modified/deleted.
 * Only dynamic config node can be add/deleted.
 * All nodes can be queried.
 */
typedef struct silc_mgmtd_if_item_s silc_mgmtd_if_req;

/*
 * --- RESPONSE ---
 * one request maybe has one or more items.
 */
typedef struct silc_mgmtd_if_item_s silc_mgmtd_if_rsp;

/*
 * --- NOTIFY ---
 */
typedef struct silc_mgmtd_if_item_s silc_mgmtd_if_notify;

/*
 * Callback function for the node tree traversal.
 */
typedef int (*silc_mgmtd_if_node_traversal_cb)(const silc_cstr parent_path, silc_mgmtd_if_node* p_node, void* data);

/*
 * mgmtd node path will be used in interface
 */
#define SILC_MGMTD_IF_PATH_SET_LGOIN_INFO	"/action/system/set-connect-login"
#define SILC_MGMTD_IF_PATH_SET_NOTIFY_PATH	"/action/system/set-notify-path"
#define SILC_MGMTD_IF_PATH_CFG_DEV_NAME		"/config/system/misc/hostname"
#define SILC_MGMTD_IF_PATH_CFG_SESS_EXP_TIME	"/config/system/mgmt/session-exp-time"


/*
 * --------------------------- Common APIs -----------------------------
 */
/*
 * Trim the trailing newline in a string
 * str : string.
 */
void silc_mgmtd_if_trim_trail_nl(char* str);

/*
 * Get the last name start pointer of a path string. For example: "/config/snmp/user", return "user"
 * path : path string.
 */
silc_cstr silc_mgmtd_if_path_get_last_name(const silc_cstr path);

/*
 * Execute a system shell command.
 * cmd : command.
 */
int silc_mgmtd_if_system(const silc_cstr cmd);

/*
 * Execute a system shell command.
 * cmd : command.
 * output_buf : buffer to store the output print.
 * p_buf_len  : output buffer length.
 * timeout_sec    : timeout_sec for this function running.
 * retry      : if command failed, retry it.
 */
int silc_mgmtd_if_exec_system_cmd(const silc_cstr cmd, silc_cstr output_buf, int *p_buf_len, int timeout_sec, silc_bool retry);

/*
 * Execute a system shell command after given timeout.
 * cmd : command.
 * timeout : timeout(s).
 */
void silc_mgmtd_if_system_cmd_later(char* cmd, int timeout);

/*
 * Create a request.
 * path : path of the root node of this request.
 * type : request type.
 * node_root_val_str: the value of the root node of this request.
 */
silc_mgmtd_if_req* silc_mgmtd_if_req_create(const silc_cstr path,
		silc_mgmtd_if_req_type type, const silc_cstr node_root_val_str);

/*
 * Delete a request.
 * p_req : the pointer of the request data structure.
 */
void silc_mgmtd_if_req_delete(silc_mgmtd_if_req* p_req);

/*
 * Add a node into the request.
 * p_req : the pointer of the request structure.
 * node_path : the node path under the root node.
 * node_val_str: the node value string.
 */
int silc_mgmtd_if_req_add_node(silc_mgmtd_if_req* p_req, const silc_cstr node_path, const silc_cstr node_val_str);

/*
 * Find node from parent.
 * p_parent : the pointer of the parent node.
 * name : the node name.
 */
silc_mgmtd_if_node* silc_mgmtd_if_req_find_node(silc_mgmtd_if_node* p_parent, const silc_cstr name);

/*
 * Reset node value.
 * p_node : the pointer of the node.
 * node_val_str : the new value.
 */
int silc_mgmtd_if_req_reset_node(silc_mgmtd_if_node* p_node, const silc_cstr node_val_str);

/*
 * Add a node into its parent and return the node.
 * p_parent : the pointer of the parent node.
 * name : the node name.
 * node_val_str : the new value.
 */
silc_mgmtd_if_node* silc_mgmtd_if_req_add_node_ext(silc_mgmtd_if_node* p_parent, const silc_cstr name, const silc_cstr node_val_str);

/*
 * Clone a request.
 * new_path : new path for the request root path.
 * p_req    : source request to be cloned.
 */
silc_mgmtd_if_req* silc_mgmtd_if_req_clone(const silc_cstr new_path, silc_mgmtd_if_req* p_req);

/*
 * Create a response.
 * path : path of the root node of this response.
 * return_str : return string of the response.
 * node_root_val_str: the value of the root node of this response.
 */
silc_mgmtd_if_rsp* silc_mgmtd_if_rsp_create(const silc_cstr path,
		const silc_cstr return_str, const silc_cstr node_root_val_str);

/*
 * Delete a response.
 * p_rsp : the pointer of the response data structure.
 */
void silc_mgmtd_if_rsp_delete(silc_mgmtd_if_rsp* p_rsp);


/*
 * Do a traversal for all the nodes in response.
 * p_rsp : the pointer of the response data structure.
 * cb_func : callback function of the node traversal.
 * cb_data : parameter of the callback function.
 */
int silc_mgmtd_if_rsp_node_traversal(silc_mgmtd_if_rsp* p_rsp, silc_mgmtd_if_node_traversal_cb cb_func, void* cb_data);

/*
 * Check response is OK.
 * p_rsp : the pointer of the response data structure.
 */
silc_bool silc_mgmtd_if_rsp_check(silc_mgmtd_if_rsp* p_rsp);

/*
 * Create a notify.
 * path : path of the root node of this notify.
 * type : request type.
 * node_root_val_str: the value of the root node of this request.
 */
silc_mgmtd_if_notify* silc_mgmtd_if_notify_create(const silc_cstr path, const silc_cstr node_root_val_str);

/*
 * Delete a notify.
 * p_req : the pointer of the notify data structure.
 */
void silc_mgmtd_if_notify_delete(silc_mgmtd_if_notify* p_notify);

/*
 * Add a node into the notify.
 * p_req : the pointer of the request structure.
 * node_path : the node path under the root node.
 * node_val_str: the node value string.
 */
int silc_mgmtd_if_notify_add_node(silc_mgmtd_if_notify* p_notify, const silc_cstr node_path, const silc_cstr node_val_str);

/*
 * Get a client type string.
 * type : client type.
 */
silc_cstr silc_mgmtd_if_get_client_type_str(silc_mgmtd_if_client_type type);

/*
 * Get client type from a string.
 * type_name : type string.
 */
silc_mgmtd_if_client_type silc_mgmtd_if_get_client_type(const silc_cstr type_name);

/*
 * Translate value string to a long variable.
 * val_str : value string.
 * p_val   : long pointer.
 */
int silc_mgmtd_if_cstr2l(const silc_cstr val_str, long *p_val);
int silc_mgmtd_if_cstr2ll(const silc_cstr val_str, long long *p_val);

int silc_mgmtd_if_cstr2ipv4addr(const silc_cstr val_str, in_addr_t *p_val);

silc_bool silc_mgmtd_if_is_server();


#define is_mgmtd_if_add_node_str_maybe(p_parent, query_node_name, str_name, val_str)	\
	do{ \
		if(query_node_name == NULL) {\
			if(0 == silc_mgmtd_if_req_add_node_ext(p_parent, str_name, val_str)) \
					goto ERROR;\
		} \
		else if(strcmp(query_node_name, str_name)==0) {\
			if(0 != silc_mgmtd_if_req_reset_node(p_parent, val_str)) \
					goto ERROR;\
		} \
	}while(0)

#define is_mgmtd_if_add_node_bool_maybe(p_parent, query_node_name, str_name, val_bool)	\
	do{ \
		char tmp_str[100]; \
		if(query_node_name == NULL) { \
			if(0 == silc_mgmtd_if_req_add_node_ext(p_parent, str_name, \
					silc_mgmtd_var_bool_to_str(val_bool , tmp_str, sizeof(tmp_str) - 1))) \
					goto ERROR;\
		} \
		else if(strcmp(query_node_name, str_name)==0) {\
			if(0 != silc_mgmtd_if_req_reset_node(p_parent, silc_mgmtd_var_bool_to_str(val_bool , tmp_str, sizeof(tmp_str) - 1))) \
					goto ERROR;\
		} \
	}while(0)

/*
 * --------------------------- Client(CLI/GUI/Internal) APIs -----------------------------
 */
/**
 * Initializae a client library
 * @param server_addr IPV4 address of remote server(Mgmtd).
 * @param listen_port remote server listen port.
 * @param idle_timeout_sec timeout_sec of connecting to remote server.
 * @param conn_timeout_sec timeout_sec of waiting for available socket conncetion.
 * @return
 */
int silc_mgmtd_if_client_init(const silc_cstr server_addr, int listen_port, int idle_timeout_sec, int conn_timeout_sec);

/**
 * shutdown managment client interface, this should be the last call to all silc_mgmtd_if_client functions
 * @param server_addr
 * @param listen_port
 * @param timeout_sec
 * @return
 */
void silc_mgmtd_if_client_shutdown(void);

/*
 * Set client information.
 * type      : client type.
 * user_name : login user name.
 * login_ip  : login IP.
 * login_port: login port.
 */
int silc_mgmtd_if_client_set_login_info(silc_mgmtd_if_client_type type,
		const silc_cstr user_name, const silc_cstr login_ip, const silc_cstr login_port);

int silc_mgmtd_if_client_set_login_info_ex(silc_mgmtd_if_client_type type,
		const silc_cstr user_name, const silc_cstr login_ip, const silc_cstr login_port,
		const silc_cstr privil, const silc_cstr proto);
/*
 * Get session expire timeout_sec.
 * p_timeout_sec : return the timeout_sec.
 */
int silc_mgmtd_if_client_get_mgmtd_session_timeout(int* p_timeout_sec);

/*
 * Set local session idle timeout_sec
 * timeout_sec : timeout_sec seconds
 */
void silc_mgmtd_if_client_set_local_session_timeout(int timeout_sec);

/*
 * Get local session idle timeout_sec
 */
int silc_mgmtd_if_client_get_local_session_timeout();


/*
 * Pop received message.
 * p_rsp_list : received message list.
 * p_seq_num  : return the message sequence number.
 */
//int silc_mgmtd_if_client_pop_msg(silc_list* p_rsp_list, uint32_t* p_seq_num);
//silc_mgmtd_conn_msg* silc_mgmtd_if_pop_msg(silc_mgmtd_conn_handler* p_handler, int timeout_sec);

void silc_mgmtd_if_client_free_msg(silc_mgmtd_msg* p_msg);
void silc_mgmtd_if_client_clear_msg(silc_mgmtd_msg* p_msg);

/*
 * Send a request and wait the response.
 * p_req : the pointer of the request structure.
 * timeout_sec: timeout_sec of sending request and waiting response.
 */
silc_mgmtd_msg* silc_mgmtd_if_client_send_req_sync(silc_mgmtd_if_req* p_req, int timeout_sec);


/*
 * Start the notify receive daemon
 */
typedef int (*silc_mgmtd_if_notify_cb)(silc_mgmtd_if_notify* p_notify, void* data);
int silc_mgmtd_if_client_start_notify_daemon(void);

/*
 * Set the notify path, the max number path can be set is SILC_MGMTD_CONN_RECV_NOTIFY_MAX
 * path : notify path
 * daemon_entry: the entry function, handle the notify.
 * data : entry data.
 */
int silc_mgmtd_if_client_set_recv_notify(const silc_cstr path,
		silc_mgmtd_if_notify_cb daemon_entry, void* data);

/*
 * ------------------------------ API for Mgmtd ---------------------------------
 */
int silc_mgmtd_if_server_init(const char* bind_addr, int listen_port, int timeout_sec);

silc_mgmtd_msg* silc_mgmtd_if_server_pop_msg(int timeout_us);

int silc_mgmtd_if_server_send_msg(silc_mgmtd_msg* p_msg);

int silc_mgmtd_if_server_send_notify(silc_mgmtd_if_notify* p_notify);

int silc_mgmtd_if_server_notify_path_set(void* conn_entry, const silc_cstr notify_path);

int silc_mgmtd_if_server_notify(silc_cstr path);

int silc_mgmtd_if_server_conn_login_set(void* conn_entry,
		silc_mgmtd_if_client_type type, const silc_cstr user_name, const silc_cstr ip, uint16_t port);

silc_mgmtd_if_client_type silc_mgmtd_if_server_get_conn_type(void* conn_entry);

silc_mgmtd_if_privilege silc_mgmtd_if_server_get_conn_privilege(void* conn_entry);

void silc_mgmtd_if_server_set_conn_privilege(void* conn_entry, silc_mgmtd_if_privilege level);

uint16_t silc_mgmtd_if_server_get_conn_id(void* conn_entry);

const silc_cstr silc_mgmtd_if_server_get_conn_index_user(uint16_t index);
const silc_cstr silc_mgmtd_if_server_get_conn_entry_user(void* conn_entry);

silc_mgmtd_if_node* silc_mgmtd_if_server_node_add(const silc_cstr node_name, const silc_cstr val_str, silc_mgmtd_if_node* p_parent_node);

int silc_mgmtd_if_server_node_set_val(silc_mgmtd_if_node* p_node, const silc_cstr new_val_str);

void silc_mgmtd_if_server_node_delete(silc_mgmtd_if_node* p_node);

/*
 * format idx,username,type,time,ip,port;idx,username,type,time,ip,port;...
 */
typedef struct silc_mgmtd_if_session_info_s
{
	silc_list node;
	char index[8];
	char type[8];
	char login_user[32];
	char login_ip[64];
	uint16_t login_port;
	char login_time[32];
}silc_mgmtd_if_session_info;
int silc_mgmtd_if_server_get_online_sessions_info(silc_list* p_list);
void silc_mgmtd_if_server_free_online_sessions_info(silc_list* p_list);

void silc_mgmtd_if_trans_privil(silc_cstr privil);

silc_cstr silc_mgmtd_if_get_req_type_str(silc_mgmtd_if_req_type type);

void silc_mgmtd_if_base64_escape(silc_cstr str);
void silc_mgmtd_if_base64_unescape(silc_cstr str);

int silc_mgmtd_if_str_escape_len(char* str);
void silc_mgmtd_if_str_escape(char* str, char* newstr);
void silc_mgmtd_if_str_unescape(char* str, char* newstr);

silc_cstr silc_mgmtd_if_get_vendor_name();
silc_bool silc_mgmtd_if_cmp_vendor_name(silc_cstr name);
int silc_mgmtd_if_get_vendor_id();
silc_bool silc_mgmtd_if_cmp_vendor_id(silc_mgmtd_vendor_id id);

void silc_mgmtd_if_product_info_set(silc_cstr product_name, int product_id, silc_cstr* vendor_list, int vendor_cnt);

#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_INTERFACE_H_ */
