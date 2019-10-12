
#ifndef SILC_MGMTD_CONNECT_H_
#define SILC_MGMTD_CONNECT_H_

#include "silc_common.h"
#include "silc_common/silc_thread.h"

#include "silc_mgmtd_lib_err.h"
#include "silc_mgmtd_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SILC_MGMTD_CONN_BUFF_MAX		SILC_MGMTD_IF_BUFFER_MAX_LEN
#define SILC_MGMTD_CONN_MSG_LEN_MAX		SILC_MGMTD_CONN_BUFF_MAX
#define SILC_MGMTD_CONN_RECV_NOTIFY_MAX	10

typedef int (*silc_mgmtd_conn_cb)(void* data, int data_len);


#define SILC_MGMTD_CONN_BUFF_MAX		SILC_MGMTD_IF_BUFFER_MAX_LEN
#define SILC_MGMTD_CONN_MSG_LEN_MAX		SILC_MGMTD_CONN_BUFF_MAX
#define SILC_MGMTD_CONN_RECV_NOTIFY_MAX	10


typedef struct silc_mgmtd_net_s
{
	silc_socket *sock;
	uint16_t		conn_index;
	uint16_t		curr_sequence;
	silc_sem			sleep_sem;
	silc_thread_id	client_thread;
	silc_mutex*		conn_lock;
	silc_list			conn_list;
	silc_list			conn_closed_list;

	// receive
	silc_sem			recv_sem;
	silc_list			msg_recv_list;

	// receive message callback
	uint16_t			msg_head_len;
	silc_mgmtd_conn_cb	check_msg_head;
	silc_mgmtd_conn_cb	seek_msg_head;
	silc_mgmtd_conn_cb	seek_msg_end;

	char				send_buff[SILC_MGMTD_CONN_BUFF_MAX];
}silc_mgmtd_net;

struct silc_mgmtd_conn_s
{
	silc_list_node	node;
	silc_mgmtd_net* p_parent;
	silc_mgmtd_if_client_type type;
	silc_mgmtd_if_privilege if_level;
	uint16_t		index;
	uint16_t		login_port;
	silc_cstr			login_user;
	silc_cstr			login_ip;
	time_t			login_time;

	uint32_t		msg_use_counter;	//used to track message usage, for connection free up
	silc_sock_conn_id	sock_conn_id;
	silc_cstr			recv_notify_path_list[SILC_MGMTD_CONN_RECV_NOTIFY_MAX];

	void* p_cur_recv_msg;

	// data receive
	silc_bool	msg_seek;
//	uint32_t recv_buff_start_pos;
//	uint32_t recv_buff_end_pos;
	uint32_t recv_buff_curr_size;
	int recv_buff_msg_start;
	char recv_buff[SILC_MGMTD_CONN_BUFF_MAX];
};



int silc_mgmtd_net_init(silc_mgmtd_net* p_handler, const char* bind_addr,
		int listen_port, int idle_timeout_sec, const silc_cstr server_addr,
		uint16_t msg_head_len,
		silc_mgmtd_conn_cb check_msg_head,
		silc_mgmtd_conn_cb seek_msg_head,
		silc_mgmtd_conn_cb seek_msg_end);

void silc_mgmtd_net_deinit(silc_mgmtd_net* p_handler);

int silc_mgmtd_net_server_start(silc_mgmtd_net* p_handler);

int silc_mgmtd_net_client_start(silc_mgmtd_net* p_handler, int conn_timeout_sec);

int silc_mgmtd_net_stop(silc_mgmtd_net* p_handler);

silc_mgmtd_msg* silc_mgmtd_net_msg_receive(silc_mgmtd_net* p_handler, int timeout_us);

/**
 * Free a message previously received. This only frees the allocation done by silc_mgmtd_net
 * @param p_msg
 */
void silc_mgmtd_net_msg_free(silc_mgmtd_msg* p_msg);

int silc_mgmtd_net_server_notify(const silc_cstr path, silc_mgmtd_net* p_handler, char *buff, int len);

int silc_mgmtd_net_conn_send(silc_mgmtd_net* p_handler, silc_mgmtd_conn* p_conn, char *buff, int len);

#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_CONNECT_H_ */
