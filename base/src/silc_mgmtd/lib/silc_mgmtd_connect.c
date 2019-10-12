
#include <signal.h>
#include "silc_common.h"
#include "silc_common/silc_thread.h"
#include "silc_mgmtd_lib_err.h"
#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_connect.h"

silc_mgmtd_msg* silc_mgmtd_net_msg_alloc(silc_mgmtd_net*p_handler, silc_mgmtd_conn* p_conn, int msg_size)
{
	silc_mgmtd_msg* p_msg;

	p_msg = malloc(sizeof(silc_mgmtd_msg));
	if(p_msg == NULL)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_OUT_OF_MEM);
		return NULL;
	}
	memset(p_msg, 0, sizeof(*p_msg));
	p_msg->data = malloc(msg_size+16);
	p_msg->conn_entry = p_conn;
	p_conn->msg_use_counter ++;
	silc_list_init(&p_msg->if_recv_items);
	silc_list_init(&p_msg->if_send_items);
	if(p_msg->data == NULL)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_OUT_OF_MEM);
		free(p_msg);
		return NULL;
	}
//	fprintf(stderr,"Alloc message %p\n", p_msg);
	return p_msg;
}

/**
 * This function is called in conn lock
 * @param p_msg
 */
void __silc_mgmtd_net_msg_free(silc_mgmtd_msg* p_msg)
{
	if(p_msg->conn_entry->msg_use_counter == 0)
	{
		SILC_ERR("Connection reference counter is already 0");
	}
	p_msg->conn_entry->msg_use_counter --;
	if(p_msg->conn_entry->msg_use_counter)
	{
		if(p_msg->conn_entry->sock_conn_id == NULL)
		{//sock already closed, free the connection
			silc_list_del(&p_msg->conn_entry->node);
			free(p_msg->conn_entry);
		}
	}
	p_msg->conn_entry = NULL;
	if(p_msg->data)
		free(p_msg->data);
	free(p_msg);
}

void silc_mgmtd_net_msg_free(silc_mgmtd_msg* p_msg)
{
	silc_mgmtd_conn* p_conn = p_msg->conn_entry;

//	fprintf(stderr, "Free  message %p\n", p_msg);
	if(p_msg->conn_entry == NULL)
	{
		fprintf(stderr, "Invalid free of message %p\n", p_msg);
		return;
	}
	silc_mutex_lock(p_conn->p_parent->conn_lock);
	__silc_mgmtd_net_msg_free(p_msg);
	silc_mutex_unlock(p_conn->p_parent->conn_lock);
}

silc_mgmtd_msg* __silc_mgmtd_net_msg_receive(silc_mgmtd_net* p_handler)
{
	silc_mgmtd_msg* p_msg;
	silc_mutex_lock(p_handler->conn_lock);
	if(silc_list_empty(&p_handler->msg_recv_list))
	{
		silc_mutex_unlock(p_handler->conn_lock);
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_WAIT_TIMEOUT);
		return NULL;
	}
	p_msg = silc_list_entry(p_handler->msg_recv_list.next, silc_mgmtd_msg, node);
	silc_list_del(&p_msg->node);
	SILC_TRACE("[%s]p_handler:%p, msg:%s\n", __func__, p_handler, p_msg->data);
	silc_mutex_unlock(p_handler->conn_lock);
	return p_msg;

}

silc_mgmtd_msg* silc_mgmtd_net_msg_receive(silc_mgmtd_net* p_handler, int timeout_us)
{
	int ret;
	silc_mgmtd_msg* p_msg;

	while(1)
	{
		p_msg = __silc_mgmtd_net_msg_receive(p_handler);
		if(p_msg)
		{//have something to return, also consume the sem with 0 delay
			silc_sem_wait(&p_handler->recv_sem, 0);
			return p_msg;
		}

		//ideally we should compensate for the timeout_us,
		//however the case where it could return early is interrupted sys call.
		ret = silc_sem_wait(&p_handler->recv_sem, timeout_us);
		if(ret!=0)
		{
			if(errno == ETIMEDOUT)
				return NULL;
		}
	}
	return NULL;
}


/*
 * np socket
 */
uint16_t silc_mgmtd_net_conn_idx_get_new(silc_mgmtd_net* p_handler)
{
	silc_mgmtd_conn *p_conn;
	silc_bool found;
	uint32_t count = 0;
	while(1)
	{
		p_handler->conn_index++;
		if(p_handler->conn_index == 0)
			p_handler->conn_index++;
		found = silc_false;
		silc_list_for_each_entry(p_conn, &p_handler->conn_list, node)
		{
			if(p_conn->index == p_handler->conn_index)
			{
				found = silc_true;
				break;
			}
		}
		if(!found)
			return p_handler->conn_index;
		count ++;
		if(count > 65535)
			break;
	}

	return 0;
}

int silc_mgmtd_net_socket_cbf_conn_new(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_socket_conn)
{
	silc_mgmtd_net* p_handler = (silc_mgmtd_net*)p_svr_arg;
	silc_mgmtd_conn *p_conn = (silc_mgmtd_conn*)malloc(sizeof(silc_mgmtd_conn));
	if(!p_conn)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}

	memset(p_conn, 0, sizeof(silc_mgmtd_conn));
	silc_mutex_lock(p_handler->conn_lock);

	p_conn->p_parent = p_handler;
	p_conn->if_level = SILC_MGMTD_IF_LEVEL_READONLY;
	p_conn->sock_conn_id = p_socket_conn;
	p_conn->index = silc_mgmtd_net_conn_idx_get_new(p_handler);
	p_conn->login_time = time(NULL);
	silc_list_add(&p_conn->node, &p_handler->conn_list);
	silc_mutex_unlock(p_handler->conn_lock);
	return 0;
}

int silc_mgmtd_net_socket_cbf_conn_close(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_socket_conn, void* p_conn_arg)
{
	silc_mgmtd_net* p_handler = (silc_mgmtd_net*)p_svr_arg;
	silc_mgmtd_conn *p_conn, *p_tmp;

	silc_mutex_lock(p_handler->conn_lock);
	silc_list_for_each_entry_safe(p_conn, p_tmp, &p_handler->conn_list, node)
	{
		if(p_conn->sock_conn_id == p_socket_conn)
		{
			silc_mgmtd_msg *p_msg, *p_tmp_msg;
			silc_list_del(&p_conn->node);
			silc_list_for_each_entry_safe(p_msg, p_tmp_msg, &p_handler->msg_recv_list, node)
			{
				silc_list_del(&p_msg->node);
				__silc_mgmtd_net_msg_free(p_msg);
			}
			if(p_conn->msg_use_counter)
			{//We still have message referencing this conn, save the conn in a temp list first
				p_conn->sock_conn_id = NULL;
				silc_list_add_tail(&p_conn->node, &p_handler->conn_closed_list);
			}
			else
			{
				free(p_conn);
			}
			silc_mutex_unlock(p_handler->conn_lock);
			return 1;
		}
	}
	silc_mutex_unlock(p_handler->conn_lock);
	return 1;
}

int silc_mgmtd_net_socket_cbf_conn_idle(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg)
{
//	silc_mgmtd_conn_socket_close_callback(p_socket, p_svr_arg, p_conn, p_conn_arg);
	//this will bring the may loop down.
	silc_socket_stop(p_socket);

	silc_mgmtd_lib_err_set(LIB_ERR_CONN_TIMED_OUT);
	//return -1 to let the caller to close the connection
	return -1;
}


void silc_mgmtd_net_conn_recv_buf_shift(silc_mgmtd_conn* p_conn, uint32_t shift_size)
{
	uint32_t tail_size;
	if(shift_size >= p_conn->recv_buff_curr_size)
	{
		p_conn->recv_buff_curr_size = 0;
		p_conn->recv_buff_msg_start = -1;
	}
	else
	{
		tail_size = p_conn->recv_buff_curr_size - shift_size;
		memmove(p_conn->recv_buff, p_conn->recv_buff + shift_size, tail_size);
		p_conn->recv_buff_curr_size = tail_size;
		p_conn->recv_buff_msg_start = -1;
	}

}

int silc_mgmtd_net_conn_msg_process(silc_mgmtd_net* p_handler, silc_mgmtd_conn* p_conn, silc_rng_buf_id p_data_rng_buf)
{
	silc_mgmtd_msg* p_msg;
	int left_len = silc_rng_buf_get_length(p_data_rng_buf);
	int copy_len;
	int msg_end_miss = 0;

	while(left_len)
	{
		if(msg_end_miss >= 100)
		{
			silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_END_NOTEXIST);
			return -1;
		}

		//first we try to copy data from ring to our buffer
		copy_len = SILC_MGMTD_CONN_BUFF_MAX - p_conn->recv_buff_curr_size -1;
		if(copy_len > left_len)
			copy_len = left_len;
		if(silc_rng_buf_pop(p_data_rng_buf, p_conn->recv_buff+p_conn->recv_buff_curr_size, copy_len) < 0)
			return -1;
		p_conn->recv_buff_curr_size += copy_len;
		p_conn->recv_buff[p_conn->recv_buff_curr_size] = 0;
		left_len -= copy_len;

		//then we try to parse as many messages as we can.
		while(1)
		{
			int msg_size;
			if(p_conn->recv_buff_msg_start < 0)
			{
				p_conn->recv_buff_msg_start = p_handler->seek_msg_head(p_conn->recv_buff, p_conn->recv_buff_curr_size);
			}
			if(p_conn->recv_buff_msg_start < 0)
				break;
			// found new header, now look for end
			if(p_conn->recv_buff_msg_start)
			{//we have garbage, need to log this.
				SILC_ERR("Garbage data found in mgmtd message %s", p_conn->recv_buff);
			}
			msg_size = p_handler->seek_msg_end(p_conn->recv_buff + p_conn->recv_buff_msg_start, p_conn->recv_buff_curr_size - p_conn->recv_buff_msg_start);
			if(msg_size <= 0) //msg_end_pos can't be 0
			{
				msg_end_miss++;
				usleep(100);
				break;
			}

			//found message end
			p_msg = silc_mgmtd_net_msg_alloc(p_handler, p_conn, msg_size);
			if(p_msg==NULL)
			{
				SILC_ERR("Failed to allocate memmory for mgmtd message size, %u", msg_size);
				silc_mgmtd_net_conn_recv_buf_shift(p_conn, p_conn->recv_buff_msg_start + msg_size);
				continue; //continue with next message
			}
			memcpy(p_msg->data, p_conn->recv_buff + p_conn->recv_buff_msg_start, msg_size);
			p_msg->data[msg_size] = 0;
			silc_mgmtd_req_dbg(p_msg, "Read", "Data: %s", p_msg->data);
			silc_list_add_tail(&p_msg->node, &p_handler->msg_recv_list);
			silc_sem_give(&p_handler->recv_sem);
			silc_mgmtd_net_conn_recv_buf_shift(p_conn, p_conn->recv_buff_msg_start + msg_size);
		}
	}

	return 0;
}

int silc_mgmtd_net_socket_cbf_conn_recv( silc_socket* p_socket, void* p_svr_arg,
		silc_sock_conn_id p_socket_conn, void* p_conn_arg, silc_rng_buf_id p_data_rng_buf)
{
	int ret = 0;
	silc_mgmtd_net* p_handler = (silc_mgmtd_net*)p_svr_arg;
	silc_mgmtd_conn* p_conn;

	silc_mutex_lock(p_handler->conn_lock);
	silc_list_for_each_entry(p_conn, &p_handler->conn_list, node)
	{
		if(p_conn->sock_conn_id == p_socket_conn)
		{
			ret = silc_mgmtd_net_conn_msg_process(p_handler, p_conn, p_data_rng_buf);
			break;
		}
	}
	silc_mutex_unlock(p_handler->conn_lock);
	return ret;
}

void silc_mgmtd_net_deinit(silc_mgmtd_net* p_handler)
{
	silc_socket_destroy(p_handler->sock);
}

#define SILC_MGMTD_UNIX_SOCKET
int silc_mgmtd_net_init(silc_mgmtd_net* p_handler, const char* bind_addr,
		int listen_port, int idle_timeout_sec, const silc_cstr server_addr,
		uint16_t msg_head_len,
		silc_mgmtd_conn_cb check_msg_head,
		silc_mgmtd_conn_cb seek_msg_head,
		silc_mgmtd_conn_cb seek_msg_end) // only client need set server_adder.
{
	silc_socket_opt sock_opt;
	char unix_domain[108] = {0};
	memset(p_handler, 0, sizeof(*p_handler));
	memset(&sock_opt, 0, sizeof(sock_opt));
	p_handler->conn_lock = silc_mutex_create();
	sock_opt.max_buf_size = SILC_MGMTD_CONN_BUFF_MAX;
	sock_opt.callback_conn_new = silc_mgmtd_net_socket_cbf_conn_new;
	sock_opt.callback_conn_close = silc_mgmtd_net_socket_cbf_conn_close;
	sock_opt.callback_data_read = silc_mgmtd_net_socket_cbf_conn_recv;
	silc_sem_init(&p_handler->recv_sem);
	silc_list_init(&p_handler->msg_recv_list);
	p_handler->conn_lock = silc_mutex_create();
	if(p_handler->conn_lock == NULL)
		return -1;

	if(idle_timeout_sec == -1) // nerver timeout
	{
		sock_opt.idle_interval.tv_sec = 10;
		sock_opt.callback_conn_idle = NULL;
	}
	else
	{
		sock_opt.idle_interval.tv_sec = idle_timeout_sec;
		sock_opt.callback_conn_idle = silc_mgmtd_net_socket_cbf_conn_idle;
	}
#ifdef SILC_MGMTD_UNIX_SOCKET
	if(server_addr)
	{
		sock_opt.o.unix_client.reconnect = silc_true;
		sprintf(unix_domain,"/var/run/silc_mgmtd_unix.domain");
		strcpy(sock_opt.o.unix_client.unix_domain, unix_domain);
		sock_opt.socket_type = SILC_SOCKET_TYPE_UNIX_CLIENT;
	}
	else
	{
		sock_opt.socket_type = SILC_SOCKET_TYPE_UNIX_SERVER;
		sock_opt.o.unix_svr.single_thread = silc_true;
		sprintf(unix_domain,"/var/run/silc_mgmtd_unix.domain");
		strcpy(sock_opt.o.unix_svr.unix_domain, unix_domain);
	}
#else
	if(server_addr)
	{
		sock_opt.port_num = listen_port;
		sock_opt.o.tcp_client.reconnect = silc_true;
		strcpy(sock_opt.o.tcp_client.remote_host, server_addr);
		sock_opt.socket_type = SILC_SOCKET_TYPE_TCP_CLIENT;
	}
	else
	{
		sock_opt.port_num = listen_port;
		sock_opt.socket_type = SILC_SOCKET_TYPE_TCP_SERVER;
		sock_opt.o.tcp_svr.single_thread = silc_true;
		if(bind_addr)
			strcpy(sock_opt.o.tcp_svr.bind_address, bind_addr);
	}
#endif
	silc_list_init(&p_handler->conn_list);
	silc_list_init(&p_handler->conn_closed_list);
	p_handler->msg_head_len = msg_head_len;
	p_handler->check_msg_head = check_msg_head;
	p_handler->seek_msg_end = seek_msg_end;
	p_handler->seek_msg_head = seek_msg_head;

	p_handler->sock = silc_socket_create(&sock_opt, p_handler);
	if(!p_handler->sock)
	{
		silc_mutex_destroy(p_handler->conn_lock);
		SILC_ERR("Create socket failed\n");
		silc_mgmtd_lib_err_set(LIB_ERR_CONN_SOCK_CREATE_FAILED);
		return -1;
	}

	if(!server_addr)
		SILC_LOG("Wait connect ... \n");

	return 0;
}

void* silc_mgmtd_net_server_thread_body(void* thread_arg)
{
	silc_mgmtd_net* p_handler = (silc_mgmtd_net*)thread_arg;

	//the np socket loop, the function will return when
	//another thread calls silc_socket_stop
	silc_socket_data_recv_loop(p_handler->sock);

	return NULL;
}


int silc_mgmtd_net_server_start(silc_mgmtd_net* p_handler)
{
	return silc_thread_create_detached(silc_mgmtd_net_server_thread_body, p_handler);
}


void* silc_mgmtd_net_client_thread_body(void* thread_arg)
{
	silc_mgmtd_net* p_handler = (silc_mgmtd_net*)thread_arg;

	//the np socket loop, the function will return when
	//another thread calls silc_socket_stop or silc_socket_destroy
	silc_socket_tcp_client_loop(p_handler->sock);

	return NULL;
}

int silc_mgmtd_net_client_start(silc_mgmtd_net* p_handler, int conn_timeout_sec)
{
	int try_cnt = conn_timeout_sec * 1000; // i.e, timeout * 10^6 / 1000

	silc_log_level_set(SILC_LOG_LEVEL_ERR);
	p_handler->client_thread = silc_thread_create(silc_mgmtd_net_client_thread_body, p_handler);

	if(p_handler->client_thread == NULL)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_CONN_CONNECT_TIMEOUT);
		return -1;
	}

	silc_sem_wait(&p_handler->sleep_sem, 100000);
	while(1)
	{
		silc_mutex_lock(p_handler->conn_lock);
		if(!silc_list_empty(&p_handler->conn_list))
		{
		//	silc_mgmtd_conn *p_conn = silc_list_entry(p_handler->conn_list.next, silc_mgmtd_conn, node);
		//	printf("[%s]create p_conn:%p\n", __func__, p_conn);
			silc_mutex_unlock(p_handler->conn_lock);
			return 0;
		}
		silc_mutex_unlock(p_handler->conn_lock);
		silc_sem_wait(&p_handler->sleep_sem, 1000);

		if (conn_timeout_sec > 0) {
			try_cnt--;
			if (try_cnt <= 0){
				break;
			}
		}
	}
	silc_mgmtd_lib_err_set(LIB_ERR_CONN_CONNECT_TIMEOUT);
	return -1;
}

int silc_mgmtd_net_stop(silc_mgmtd_net* p_handler)
{
	silc_socket_stop(p_handler->sock);
	silc_thread_wait_destroy(p_handler->client_thread);
	return 0;
}


int silc_mgmtd_net_server_notify(const silc_cstr path,
		silc_mgmtd_net* p_handler, char *buff, int len)
{
	int ret, loop;
	silc_mgmtd_conn* p_conn;

	//TODO: This is hack, silc_mgmtd_if_server_init may not even be called before this is called.
	//Need to fix properly, but we don't have time right now.
	//***************Hack start
	if(p_handler->conn_lock == NULL)
		return 0;
	//*****************Hack finish.
	silc_mutex_lock(p_handler->conn_lock);
	silc_list_for_each_entry(p_conn, &p_handler->conn_list, node)
	{
		//printf("[%s]path:%s, p_conn path:%s\n", __func__, path, p_conn->recv_notify_path);
		for(loop=0; loop<SILC_MGMTD_CONN_RECV_NOTIFY_MAX; loop++)
		{
			if(!p_conn->recv_notify_path_list[loop])	// the end
				break;
			if(strcmp(p_conn->recv_notify_path_list[loop], path) == 0)
			{
				SILC_TRACE("[%s] send:%s", __func__, buff);
				ret = silc_sock_conn_send(p_conn->sock_conn_id, buff, len);
				if(ret < 0)
					silc_mgmtd_lib_err_set(LIB_ERR_CONN_SEND_FAILED);
			}
		}
	}
	silc_mutex_unlock(p_handler->conn_lock);
	return 0;
}

#if 0
int silc_mgmtd_net_client_send(silc_mgmtd_net* p_handler, char *buff, int len)
{
	int ret;
	silc_mgmtd_conn* p_conn;

	silc_mutex_lock(p_handler->conn_lock);
	if(silc_list_empty(&p_handler->conn_list))
	{
		silc_mutex_unlock(p_handler->conn_lock);
		return -1;
	}

	p_conn = silc_list_entry(p_handler->conn_list.next, silc_mgmtd_conn, node);

	if(!p_conn->sock_conn_id)
	{
		silc_mutex_unlock(p_handler->conn_lock);
		silc_mgmtd_lib_err_set(LIB_ERR_CONN_ID_IS_NULL);
		return -1;
	}

	ret = silc_sock_conn_send(p_conn->sock_conn_id, buff, len);
	if(ret < 0)
		silc_mgmtd_lib_err_set(LIB_ERR_CONN_SEND_FAILED);
	silc_mutex_unlock(p_handler->conn_lock);
	return ret;
}
#endif
int __silc_mgmt_net_conn_send(silc_mgmtd_net* p_handler, silc_mgmtd_conn* p_conn, char *buff, int len)
{
	int ret;
	if(p_conn->sock_conn_id == NULL)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_CONN_CLOSED);
		return -1;
	}
	ret = silc_sock_conn_send(p_conn->sock_conn_id, buff, len);
	if(ret < 0)
		silc_mgmtd_lib_err_set(LIB_ERR_CONN_SEND_FAILED);
	SILC_TRACE("[%s]p_handler:%p, p_conn:%p, send: %s\n", __func__, p_handler, p_conn, buff);
	return ret;

}

int silc_mgmtd_net_conn_send(silc_mgmtd_net* p_handler, silc_mgmtd_conn* p_conn, char *buff, int len)
{
	int ret = 0;
	silc_mutex_lock(p_handler->conn_lock);
	if(p_conn == NULL)
	{//client send
		if(silc_list_empty(&p_handler->conn_list))
		{
			silc_mgmtd_lib_err_set(LIB_ERR_CONN_CLOSED);
			silc_mutex_unlock(p_handler->conn_lock);
			return -1;
		}
		p_conn = silc_list_entry(p_handler->conn_list.next, silc_mgmtd_conn, node);
	}

	ret = __silc_mgmt_net_conn_send(p_handler, p_conn, buff, len);
	silc_mutex_unlock(p_handler->conn_lock);
	return ret;
}

