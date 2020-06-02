/*************************************************************************
 *
 * File Name: console_manager.c
 * Author: Jeff
 * Mail: jeff.zheng@silicom.co.il
 * Created Time: Tue 03 Apr 2018 02:24:09 PM HKT
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <poll.h> /* poll function */
#include <limits.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <stdarg.h>
#include "console_manager_cmdline.h"
#include "is_con_mgr_mgmtd.h"
#include "silc_common.h"

#define ADDRESS     "/var/run/console_manager_socket"  /* addr to connect for socket*/
#define IS_CON_MGR_TTY_BUF_SIZE 1024
#define CONSOLE_MANAGER_SERIAL_SOURCE_1 "/dev/uart-sim"
#define CONSOLE_MANAGER_SERIAL_SOURCE_2 "/dev/ttyMV1"
#define CONSOLE_MANAGER_SERIAL_OUTPUT   "/dev/ttyMV0"
#define CONSOLE_MANAGER_SERIAL_HOST_LOG  "/var/log/host_serial.log"

#define HOST_LOG_DEFAULT_MAX_SIZE        50*1024*1024
#define HOST_LOG_DEFAULT_ROTATE_NUM      10
#define HOST_LOG_CONFIGURE_ROTATE_BIN "/etc/rsyslog_rotation.sh"

#ifdef DEBUG
#define CONSOLE_MANAGER_SERIAL_UBMC_LOG  "/var/log/ubmc_serial.log"
#endif

#if 0
#define con_mgr_dump(buf, siz, dev_no)  \
    do { \
       char prn_buf[1024]; \
       silc_hex_dump_str(buf, siz, prn_buf, sizeof(prn_buf)); \
       printf("%u->%u\n%s\n", dev_no, siz, prn_buf); \
    }while(0)
#else
#define con_mgr_dump(buf, siz, dev_no)
#endif


#define console_manager_lock_file   "/var/run/console_manager.lock"/*only a single instance application*/

enum
{
	serial_source_in_pollfd = 0, serial_output_pollfd, socket_pollfd, to_client_pollfd,/*its postion cannot be changed ,others can*/
	pollfd_max,
};

enum
{
	message_socket_client_exist_tips = 0,
	message_socket_client_kick_out_tips,
	message_socket_client_switch_serial_source_tips,
	message_switching_uart_sim_serial_source_tips,
	message_switching_tty_n_1_serial_source_tips,
	message_input_cr_nl_tips, //input char \r and \n
	message_tips_max,
};

static char *console_manager_msg[message_tips_max] = {
		"\r\nThere is an existing socket connection,please kick it out before creating a new connection.\r\n",
		"\r\nThe existing socket connection is disconneted. \r\n",
		"\r\nOnly host serial source supports socket connection,socket connection will be closed while switching serial source from HOST to UBMC.\r\n",
		"\r\n======Switching to UBMC console======\r\n",
		"\r\n======Switching to HOST console======\r\n", "\r\n" };

enum
{
	CMD_CTRL_X = 0,
	CMD_CTRL_EXIT,
	CMD_NONE,
};

enum
{
	BIT_IS_KICK_CON_FORCE_EXIT_STATUS = 0, BIT_IS_KICK_CON_SELF_EXIT_STATUS,

};

enum
{
	BIT_READ_FROM_SERIAL_OUT_STATUS = 0,
	BIT_READ_FROM_SERIAL_SOURCE_STATUS,
	BIT_SOCKET_ACCEPT_STATUS,
	BIT_READ_FROM_CLIENT_SOCKET_STATUS,
	BIT_KICK_OUT_CLIENT_SOCKET_STATUS,
};

enum
{
	IS_CON_MGR_ERR_FAIL_CREATE_SHM = 0,
	IS_CON_MGR_ERR_FAIL_CREATE_MSG_ID,
	IS_CON_MGR_ERR_SOCKET_CLIENT_NO_EXIST,
	IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED,
	IS_CON_MGR_ERR_NULL,
	IS_CON_MGR_ERR_ALLOC_MEM_FIALED,
	IS_CON_MGR_ERR_SERVER_SOCKET,
	IS_CON_MGR_ERR_SERVER_BIND,
	IS_CON_MGR_ERR_SERVER_LISTEN,
	IS_CON_MGR_ERR,
};
#define CONSOLE_DEV_NAME_MAXSIZE 64
static char console_manager_serial_source_2[CONSOLE_DEV_NAME_MAXSIZE] = {0};
static char console_manager_serial_output[CONSOLE_DEV_NAME_MAXSIZE] = {0};
struct serial_source
{
	char * serial_in_buf;
	char * serial_in_socket_buf;
	char * serial_out_buf;
	char * client_socket_buf;
	char * log_buf;
	uint serial_in_bufpos;
	uint serial_out_bufpos;
	uint serial_in_socket_bufpos;
	uint client_socket_bufpos;
	uint log_bufpos;
	struct pollfd clients[pollfd_max];
	int32_t log_fd; //for host serial log
	int32_t is_establish_con;
	int32_t is_kick_con;
	int32_t msgid;
	struct mesg_buffer *queue_msg;
	long console_output_msg_tips_st;
};

typedef struct _serial_config_paramter
{
	bool com_hwflowctrl;
	bool com_swflowctrl;
	uint com_speed;
	uint com_data;
	uint com_stopbits;
	char com_parity[32];
} serial_config_paramter;

static struct serial_source g_con_mgr_serial_dev[serial_s_max];
static int32_t current_serial_source;

static is_console g_is_console;
static uint host_log_file_max_size = HOST_LOG_DEFAULT_MAX_SIZE;
static uint host_log_rotate_num = HOST_LOG_DEFAULT_ROTATE_NUM;
static serial_config_paramter serial_config_param;



/*
 *Shared Memory
 */
static int32_t *shm_p;

int32_t con_mgr_tty_set_interface_attribs(int32_t fd, int32_t speed, int32_t parity, int32_t should_block,
		int32_t is_output);
//void console_manager_tty_set_blocking (int32_t fd, int32_t should_block);
//int32_t console_manager_tty_set_char_break(int32_t fd ,int32_t mode);
int32_t console_manager_socket_init(int32_t *server_socket_fd);
int32_t console_manager_kick_out_client_socket(int32_t serial_dev_no, int32_t msg_type);
void console_manager_output_switching_serial_source_msg_tips(int32_t serial_dev_no);
int32_t console_manager_host_log_config_load(void);
int32_t console_manager_tty_set_serial_config(int32_t fd, serial_config_paramter * p_serial_config_param);
int32_t console_manager_flush_to_serial_source(int32_t serial_dev_no, char *buf, uint * bufpos);


static int g_console_manager_bios_boot_wait_count = 0;

void console_manager_chk_bios_boot_wait(char* serial_buf, int buf_len)
{
	uint32_t loop;

	//the bios is sending us 1 byte every half second, so if buf_len is larger than 2, ignore
	if(buf_len > 1)
	{
		if(serial_buf[buf_len-1] == 'c')
			g_console_manager_bios_boot_wait_count = 1;
		else
			g_console_manager_bios_boot_wait_count = 0;
		return;
	}

	if(serial_buf[0] == 'c')
	{
		g_console_manager_bios_boot_wait_count ++;
		if(g_console_manager_bios_boot_wait_count > 4)
		{
			uint32_t wlen=1;
			//printf("Answer bios\n");
			console_manager_flush_to_serial_source(serial_s_tty_n_1, "a", &wlen);
			return;
		}
	}
	else
	{
		g_console_manager_bios_boot_wait_count =0;
	}
}
int32_t console_manager_serial_source_init(int32_t serial_dev_no, int32_t serial_source_fd, int32_t serial_out_fd,
		int32_t socket_fd, int32_t log_fd)
{
	char * tmp_ptr = NULL;

	if ((tmp_ptr = (char *) malloc(sizeof(char) * IS_CON_MGR_TTY_BUF_SIZE)) == NULL)
	{
		return IS_CON_MGR_ERR_ALLOC_MEM_FIALED;
	}

	g_con_mgr_serial_dev[serial_dev_no].serial_in_buf = tmp_ptr;

	if ((tmp_ptr = (char *) malloc(sizeof(char) * IS_CON_MGR_TTY_BUF_SIZE)) == NULL)
	{
		free(g_con_mgr_serial_dev[serial_dev_no].serial_in_buf);

		return IS_CON_MGR_ERR_ALLOC_MEM_FIALED;
	}

	g_con_mgr_serial_dev[serial_dev_no].serial_out_buf = tmp_ptr;

	memset(g_con_mgr_serial_dev[serial_dev_no].serial_in_buf, 0, IS_CON_MGR_TTY_BUF_SIZE);
	memset(g_con_mgr_serial_dev[serial_dev_no].serial_out_buf, 0, IS_CON_MGR_TTY_BUF_SIZE);
	g_con_mgr_serial_dev[serial_dev_no].serial_in_bufpos = 0;
	g_con_mgr_serial_dev[serial_dev_no].serial_out_bufpos = 0;

	g_con_mgr_serial_dev[serial_dev_no].clients[serial_source_in_pollfd].fd = serial_source_fd;
	g_con_mgr_serial_dev[serial_dev_no].clients[serial_source_in_pollfd].events = POLLIN;
	g_con_mgr_serial_dev[serial_dev_no].clients[serial_output_pollfd].fd = serial_out_fd;
	g_con_mgr_serial_dev[serial_dev_no].clients[serial_output_pollfd].events = POLLIN;

	g_con_mgr_serial_dev[serial_dev_no].console_output_msg_tips_st = 0;

	if (serial_dev_no == serial_s_tty_n_1)
	{
		g_con_mgr_serial_dev[serial_dev_no].log_fd = log_fd;

		g_con_mgr_serial_dev[serial_dev_no].clients[socket_pollfd].fd = socket_fd;
		g_con_mgr_serial_dev[serial_dev_no].clients[socket_pollfd].events = POLLIN;

		if ((tmp_ptr = (char *) malloc(sizeof(char) * IS_CON_MGR_TTY_BUF_SIZE)) == NULL)
		{

			free(g_con_mgr_serial_dev[serial_dev_no].serial_in_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].serial_out_buf);

			return IS_CON_MGR_ERR_ALLOC_MEM_FIALED;
		}

		g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_buf = tmp_ptr;

		memset(g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_buf, 0, IS_CON_MGR_TTY_BUF_SIZE);
		g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_bufpos = 0;

		if ((tmp_ptr = (char *) malloc(sizeof(char) * IS_CON_MGR_TTY_BUF_SIZE)) == NULL)
		{
			return IS_CON_MGR_ERR_ALLOC_MEM_FIALED;
		}

		g_con_mgr_serial_dev[serial_dev_no].client_socket_buf = tmp_ptr;
		memset(g_con_mgr_serial_dev[serial_dev_no].client_socket_buf, 0, IS_CON_MGR_TTY_BUF_SIZE);
		g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos = 0;

		if ((tmp_ptr = (char *) malloc(sizeof(char) * IS_CON_MGR_TTY_BUF_SIZE)) == NULL)
		{

			free(g_con_mgr_serial_dev[serial_dev_no].serial_in_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].serial_out_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].client_socket_buf);

			return IS_CON_MGR_ERR_ALLOC_MEM_FIALED;
		}

		g_con_mgr_serial_dev[serial_dev_no].log_buf = tmp_ptr;

		memset(g_con_mgr_serial_dev[serial_dev_no].log_buf, 0, IS_CON_MGR_TTY_BUF_SIZE);
		g_con_mgr_serial_dev[serial_dev_no].log_bufpos = 0;

		if ((tmp_ptr = (char *) malloc(sizeof(struct mesg_buffer))) == NULL)
		{

			free(g_con_mgr_serial_dev[serial_dev_no].serial_in_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].serial_out_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].client_socket_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].log_buf);

			return IS_CON_MGR_ERR_ALLOC_MEM_FIALED;
		}
		g_con_mgr_serial_dev[serial_dev_no].queue_msg = (struct mesg_buffer *) tmp_ptr;
		/*
		 *msgget creates a message queue and returns identifier
		 */
		g_con_mgr_serial_dev[serial_dev_no].msgid = msgget(serial_source_2_msg_queue_key, 0666 | IPC_CREAT);
		if (g_con_mgr_serial_dev[serial_dev_no].msgid < 0)
		{
			return IS_CON_MGR_ERR_FAIL_CREATE_MSG_ID;
		}

		//physical serial device must be initialized
		con_mgr_tty_set_interface_attribs(serial_source_fd, B115200, 0, 0, 0);
//       console_manager_tty_set_blocking (serial_source_fd, 0);
		console_m_info("console_manager init socket serial_source_2 sucessfully \n");

	}
	else
	{

#ifdef DEBUG
		g_con_mgr_serial_dev[serial_dev_no].log_fd = log_fd;

		if((tmp_ptr = (char *)malloc(sizeof(char) * IS_CON_MGR_TTY_BUF_SIZE )) == NULL)
		{

			free(g_con_mgr_serial_dev[serial_dev_no].serial_in_buf);
			free(g_con_mgr_serial_dev[serial_dev_no].serial_out_buf);

			return IS_CON_MGR_ERR_ALLOC_MEM_FIALED;
		}

		g_con_mgr_serial_dev[serial_dev_no].log_buf = tmp_ptr;

		memset(g_con_mgr_serial_dev[serial_dev_no].log_buf,0,IS_CON_MGR_TTY_BUF_SIZE);
		g_con_mgr_serial_dev[serial_dev_no].log_bufpos = 0;
#endif
		con_mgr_tty_set_interface_attribs(serial_out_fd, B115200, 0, 0, 1);
//        console_manager_tty_set_blocking (serial_out_fd, 0);
		console_m_info("console_manager init socket serial_source_1 sucessfully \n");
	}

	g_con_mgr_serial_dev[serial_dev_no].is_establish_con = 0;
	g_con_mgr_serial_dev[serial_dev_no].is_kick_con = 0;

	return IS_CON_MGR_ERR_NULL;

}



int32_t con_mgr_tty_set_interface_attribs(int32_t fd, int32_t speed, int32_t parity, int32_t should_block,
		int32_t is_output)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0)
	{
		console_m_err("console_manager error from tcgetattr \n");
		return -1;
	}

//	printf("%s fd %3u before, attr iflag %08x oflag %08x\n", is_output ? "out" : "in ", fd, tty.c_iflag, tty.c_oflag);

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);
#if 0
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;// disable break processing
	tty.c_lflag = 0;// no signaling chars, no echo,
					// no canonical processing
	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;// 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);// shut off xon/xoff ctrl

	tty.c_iflag &= ~(INLCR | ICRNL);//disable cr convert to newline or newline convert to cr
	tty.c_oflag &= ~(ONLCR | OCRNL);//disable cr convert to newline or newline convert to cr

//    if(!is_output)
//    	tty.c_oflag |= ONLRET;//don't output CR
//    else
//    	tty.c_iflag |= IGNCR; //ignore CR from output

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
									// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);// shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
#else

	/* input modes - clear indicated ones giving: no break, no CR to NL,
	 no parity check, no strip char, no start/stop output (sic) control */
	tty.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	/* output modes - clear giving: no post processing such as NL to CR+NL */
	tty.c_oflag &= ~(OPOST);

	/* control modes - set 8 bit chars */
	tty.c_cflag |= (CS8);

	/* local modes - clear giving: echoing off, canonical off (no erase with
	 backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) */
	tty.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

#endif
	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		console_m_err("console_manager error from tcsetattr \n");
		return -1;
	}
//	printf("%s fd %3u after , attr iflag %08x oflag %08x\n", is_output ? "out" : "in ", fd, tty.c_iflag, tty.c_oflag);
	return 0;
}

int32_t console_manager_serial_source_host_restart()
{
	int32_t old_fd, new_fd;

	old_fd = g_con_mgr_serial_dev[serial_s_tty_n_1].clients[serial_source_in_pollfd].fd;
	close(old_fd);

	usleep(10000);      //10ms

	new_fd = open(console_manager_serial_source_2, O_RDWR | O_NOCTTY | O_SYNC,S_IRWXU|S_IRWXO);
	if (new_fd < 0)
	{
		console_m_err("Fail to restart serial source host");
		console_m_err("Can't open %s: %s\n", console_manager_serial_source_2, strerror (errno));
		return IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED;
	}

	g_con_mgr_serial_dev[serial_s_tty_n_1].clients[serial_source_in_pollfd].fd = new_fd;
	console_m_notice("restart serial source host sucessfully");
	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_tty_set_serial_config(int32_t fd, serial_config_paramter * p_serial_config_param)
{
	int32_t speed;
	struct termios tty;

	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0)
	{
		console_m_err("tcgetattr returned error %s\n", strerror(errno));
		return -1;
	}

	tty.c_cflag &= ~CSIZE;
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars

	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
									// no canonical processing
	tty.c_cc[VMIN] = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(INLCR | ICRNL); //disable cr convert to newline or newline convert to cr

	tty.c_cflag |= (CLOCAL | CREAD);            // ignore modem controls,
												// enable reading

	switch (p_serial_config_param->com_speed)
	{
	case 9600:
		speed = B9600;
		break;
	case 38400:
		speed = B38400;
		break;
	case 115200:
		speed = B115200;
		break;
	default:
		speed = B115200;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	if (p_serial_config_param->com_swflowctrl)
	{
		tty.c_iflag |= IXON | IXOFF | IXANY;
	}
	else
	{
		tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	}

	if (p_serial_config_param->com_hwflowctrl)
	{
		tty.c_cflag |= CRTSCTS;
	}
	else
	{
		tty.c_cflag &= ~CRTSCTS;
	}

	switch (p_serial_config_param->com_data)
	{
	case 5:
		tty.c_cflag |= CS5;
		break;
	case 6:
		tty.c_cflag |= CS6;
		break;
	case 7:
		tty.c_cflag |= CS7;
		break;
	case 8:
		tty.c_cflag |= CS8;
		break;
	default:
		tty.c_cflag |= CS8;
		break;
	}

	if (!strcmp(p_serial_config_param->com_parity, "none"))
	{
		tty.c_cflag &= ~PARENB;									//none
	}
	else if (!strcmp(p_serial_config_param->com_parity, "even"))
	{
		tty.c_cflag |= PARENB;									//even
		tty.c_cflag &= ~PARODD;
	}
	else if (!strcmp(p_serial_config_param->com_parity, "odd"))
	{
		tty.c_cflag |= PARENB;									//odd
		tty.c_cflag |= ~PARODD;
	}
	else
	{
		tty.c_cflag &= ~PARENB;									//none
	}

	switch (p_serial_config_param->com_stopbits)
	{
	case 1:
		tty.c_cflag &= ~CSTOPB;									//1bit stop bit
		break;
	case 2:
		tty.c_cflag |= CSTOPB; //2bit stop bit
		break;
	default:
		tty.c_cflag &= ~CSTOPB;
		break;
	}

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		console_m_err("tcsetattr returned error %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int32_t console_manager_host_serial_config_load()
{
	is_config_console *p_config;
	p_config = g_is_console.p_config;

	serial_config_param.com_data = p_config->com_data;
	serial_config_param.com_speed = p_config->com_speed;
	serial_config_param.com_stopbits = p_config->com_stopbits;
	serial_config_param.com_hwflowctrl = p_config->com_hwflowctrl;
	serial_config_param.com_swflowctrl = p_config->com_swflowctrl;
	strncpy(serial_config_param.com_parity, p_config->com_parity, 32);

	console_m_notice(
			"[ com_speed :%d ,com_data :%d com_stopbits :%d ,com_hwflowctrl :%s ,com_swflowctrl :%s ,com_parity :%s]",
			serial_config_param.com_speed, serial_config_param.com_data, serial_config_param.com_stopbits,
			serial_config_param.com_hwflowctrl ? "enable" : "disable",
			serial_config_param.com_swflowctrl ? "enable" : "disable", serial_config_param.com_parity);
	return IS_CON_MGR_ERR_NULL;
}

bool console_manager_get_param_type_changed_status(is_console * p_console, uint *host_status)
{
	bool tmp_status = false;
	is_config_console *p_config;
	serial_config_paramter *p_serial_config;
	bool config_changed = false;

	p_config = g_is_console.p_config;
	p_serial_config = &serial_config_param;

	if (is_cons_mgr_mgmtd_get_param_changed_status(p_console))
	{
		if (p_config->log_rotate_num != host_log_rotate_num
				|| (p_config->log_rotate_size * 1024 * 1024) != host_log_file_max_size)
		{
			set_bit_status(*host_status, CONSOLE_CONFIG_TYPE_STATUS_LOG_ROTATION);
			config_changed = true;
		}

		if (p_config->com_data != p_serial_config->com_data || p_config->com_speed != p_serial_config->com_speed
				|| p_config->com_stopbits != p_serial_config->com_stopbits
				|| p_config->com_hwflowctrl != p_serial_config->com_hwflowctrl
				|| p_config->com_swflowctrl != p_serial_config->com_swflowctrl
				|| strcmp(p_config->com_parity, p_serial_config->com_parity))
		{
			set_bit_status(*host_status, CONSOLE_CONFIG_TYPE_STATUS_HOST_SERIAL);
			config_changed = true;
		}

		if(config_changed)
			console_m_notice("Host console configuration changed.");
		else
		{
#ifdef DEBUG
			console_m_notice("Host console configuration didn't change");
#endif
		}

		tmp_status = true;
	}

	return tmp_status;
}

int32_t console_manager_mgmtd_config_status_check(int32_t serial_dev_no)
{
	int32_t host_serial_fd;
	uint host_param_st = 0;

	/*
	 * now mgmtd config is only supporting for host
	 * only the value of g_is_console.is_initalized true,can we check it
	 */
	if (g_is_console.is_initalized && console_manager_get_param_type_changed_status(&g_is_console, &host_param_st))
	{
		printf("console_manager_mgmtd_config_status_check \n");
		if (get_bit_status(host_param_st, CONSOLE_CONFIG_TYPE_STATUS_LOG_ROTATION))
		{
			console_manager_host_log_config_load();

		}

		if (get_bit_status(host_param_st, CONSOLE_CONFIG_TYPE_STATUS_HOST_SERIAL))
		{
			console_manager_host_serial_config_load();
			/*
			 *reopen serial before setting tty parameters
			 */
			console_manager_serial_source_host_restart();
			host_serial_fd = g_con_mgr_serial_dev[serial_s_tty_n_1].clients[serial_source_in_pollfd].fd;
			console_manager_tty_set_serial_config(host_serial_fd, &serial_config_param);
		}

	}

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_flush_to_serial_source(int32_t serial_dev_no, char *buf, uint * bufpos)
{
	int32_t todo, done;
	int32_t serial_in_fd;
	char prn_buf[1024];

	serial_in_fd = g_con_mgr_serial_dev[serial_dev_no].clients[serial_source_in_pollfd].fd;

	con_mgr_dump(buf, *bufpos, serial_dev_no);

	todo = *bufpos;
	while (todo > 0)
	{
		done = write(serial_in_fd, buf, todo);
		if (done > 0)
		{
			todo -= done;
			*bufpos -= done;
		}
		if (done < 0 && errno != EINTR)
			break;
	}
	console_m_debug("End pos[%d] \n", *bufpos);

	return 0;

}

int32_t console_manager_flush_to_input(int32_t serial_dev_no)
{
	uint *serial_out_bufpos;
	char * serial_out_buf;

	serial_out_buf = g_con_mgr_serial_dev[serial_dev_no].serial_out_buf;
	serial_out_bufpos = &(g_con_mgr_serial_dev[serial_dev_no].serial_out_bufpos);

	console_manager_flush_to_serial_source(serial_dev_no, serial_out_buf, serial_out_bufpos);
	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_flush_to_socket(int32_t serial_dev_no, char *buf, uint * bufpos)
{

	int32_t todo, done;
	int32_t to_client_fd;
	to_client_fd = g_con_mgr_serial_dev[serial_dev_no].clients[to_client_pollfd].fd;

	console_m_debug("pos[%d] \n", *bufpos);
	todo = *bufpos;
	while (todo > 0)
	{
		done = write(to_client_fd, buf, todo);
		if (done > 0)
		{
			todo -= done;
			*bufpos -= done;
		}
		if (done < 0 && errno != EINTR)
			break;
	}
	console_m_debug("End pos[%d] \n", *bufpos);

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_flush_to_serial_out(int32_t serial_dev_no, char *buf, uint * bufpos)
{
	int32_t todo, done;
	int32_t serial_out_fd;
	serial_out_fd = g_con_mgr_serial_dev[serial_dev_no].clients[serial_output_pollfd].fd;

	console_m_debug("pos[%d] \n", *bufpos);

	con_mgr_dump(buf, *bufpos, serial_dev_no);

	todo = *bufpos;
	while (todo > 0)
	{
		done = write(serial_out_fd, buf, todo);
		if (done > 0)
		{
			todo -= done;
			*bufpos -= done;
		}
		if (done < 0 && errno != EINTR)
			break;
	}

	console_m_debug("End pos[%d] \n", *bufpos);

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_host_log_config_load()
{
	is_config_console *p_config;
	p_config = g_is_console.p_config;

	/*
	 *the unit of file size is MB
	 */
	host_log_file_max_size = p_config->log_rotate_size * 1024 * 1024;
	host_log_rotate_num = p_config->log_rotate_num;
	console_m_notice("host log：%s num:%d size:%d \n", p_config->is_log_to_file ? "enable" : "disable",
			p_config->log_rotate_num, p_config->log_rotate_size);

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_make_rsyslog_rotation(int32_t serial_dev_no, int32_t *fd_p)
{
	int32_t log_fd;
	int32_t log_offset;
	char tmp[128];

	log_fd = g_con_mgr_serial_dev[serial_dev_no].log_fd;
	log_offset = lseek(log_fd, 0, SEEK_CUR);

	/*
	 *making rsyslog rotation
	 */
	if (log_offset > host_log_file_max_size)
	{
		close(log_fd);
		sprintf(tmp, "%s %s %d", HOST_LOG_CONFIGURE_ROTATE_BIN,
		CONSOLE_MANAGER_SERIAL_HOST_LOG, host_log_rotate_num);
		system(tmp);
		console_m_notice("execute %s", HOST_LOG_CONFIGURE_ROTATE_BIN);

		/*
		 *open serial log file again despite of the same file name
		 */
		g_con_mgr_serial_dev[serial_dev_no].log_fd = open(CONSOLE_MANAGER_SERIAL_HOST_LOG,
		O_CREAT | O_RDWR | O_APPEND);
		*fd_p = g_con_mgr_serial_dev[serial_dev_no].log_fd;

		if (g_con_mgr_serial_dev[serial_dev_no].log_fd < 0)
		{
			console_m_err("!!! Fail to create new host serial log file(%s) \n", CONSOLE_MANAGER_SERIAL_HOST_LOG);
		}
		else
		{
			if (chmod(CONSOLE_MANAGER_SERIAL_HOST_LOG, 0644) < 0)
				console_m_warn("console_manager cannot change file(%s) permission to 0644 \n",
						CONSOLE_MANAGER_SERIAL_HOST_LOG);
			else
				console_m_notice("console_manager change file(%s) permission to 0644 \n",
						CONSOLE_MANAGER_SERIAL_HOST_LOG);
		}

	}
	else
	{

		*fd_p = log_fd;

	}

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_flush_to_log(int32_t serial_dev_no, char *buf, uint * bufpos)
{

	int32_t todo, done;
	int32_t log_fd;

	console_manager_make_rsyslog_rotation(serial_dev_no, &log_fd);
	console_m_debug("pos[%d] \n", *bufpos);
	//check host
	todo = *bufpos;
	//console_manager_chk_bios_boot_wait(buf,todo);
	while (todo > 0)
	{
		done = write(log_fd, buf, todo);
		if (done > 0)
		{
			todo -= done;
			*bufpos -= done;
		}
		if (done < 0 && errno != EINTR)
			break;
	}

	console_m_debug("End pos[%d] \n", *bufpos);

	return IS_CON_MGR_ERR_NULL;

}


#define BIOS_BOOT_LOG1 "Do you want to recovery ROM part? (Caution: Not every OS is bootable in Recovery Mode.)"
#define BIOS_BOOT_LOG2 "YES,this choice will flash Rom part."
#define BIOS_BOOT_LOG3 "NO, abort Recovery mode and restart system."
#define BIOS_BOOT_LOG4 "      Update Progress: Completed"  /*32Byte*/
#define BIOS_UPGRADE_STARTED "/tmp/h2o_bios_upgrade_started"
#define BIOS_UPGRADE_DONE "/tmp/h2o_bios_upgrade_done"
#define BIOS_UPGRADE_TIMEOUT "/tmp/h2o_bios_upgrade_timeout"
bool console_manager_match_str(const char *str,char *buf,uint buf_len)
{

	uint i = 0,j = 0,buf_pos = 0;
	uint str_len = 0;
	str_len = strlen(str);
	uint32_t* str_pos = NULL;
	str_pos = &g_is_console.match_str.match_str_pos;
    //console_m_debug("len is %d buf is %s \n",buf_len,buf);
	while(i < buf_len)
	{
		j = i;
		while(buf_pos + j < buf_len)
		{
			//console_m_debug("buf_pos is %d char is %c str is %c *str_pos is %d\n",buf_pos + j,buf[buf_pos + j],str[*str_pos],*str_pos);
			if(str[*str_pos] == buf[buf_pos + j])
			{
				(*str_pos) ++;
				j ++;
				if(*str_pos == str_len)
				{
					//console_m_debug("localtion is %d \n",buf_pos);
					*str_pos = 0;
					return 1;
				}
				if(buf_pos + j >= buf_len)
				{
					/*current has no complete string ,so just return to find in next buf*/
					return 0;
				}
				//console_m_debug("match char %c str_pos is %d \n",buf[buf_pos + j - 1],*str_pos);
			}
			else
			{
				*str_pos = 0;
				j = 0;
				buf_pos ++;
			}

		}
		i ++ ;
	}
	return 0;
}
int32_t console_manager_flush_to_output(int32_t serial_dev_no)
{
	uint *serial_in_bufpos;
	uint *serial_in_socket_bufpos;
	uint *log_bufpos;
	uint wlen = 0 ,buf_len = 0;
	bool is_matched = 0;
	char bios_boot_type = 0;
	char * serial_in_buf;
	char * serial_in_socket_buf;
	char * log_buf;
	is_config_console *p_config;
	p_config = g_is_console.p_config;

	serial_in_bufpos = &(g_con_mgr_serial_dev[serial_dev_no].serial_in_bufpos);
	serial_in_buf = g_con_mgr_serial_dev[serial_dev_no].serial_in_buf;
	console_manager_flush_to_serial_out(serial_dev_no, serial_in_buf, serial_in_bufpos);

	if (g_con_mgr_serial_dev[serial_dev_no].is_establish_con)
	{

		serial_in_socket_bufpos = &(g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_bufpos);
		serial_in_socket_buf = g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_buf;
		console_manager_flush_to_socket(serial_dev_no, serial_in_socket_buf, serial_in_socket_bufpos);
	}

	/*
	 *only write log enable can write to log file
	 */
#ifndef DEBUG
	if (p_config->is_log_to_file)
#endif
	{
		log_bufpos = &(g_con_mgr_serial_dev[serial_s_tty_n_1].log_bufpos);
		buf_len = *log_bufpos;
		log_buf = g_con_mgr_serial_dev[serial_s_tty_n_1].log_buf;
		console_manager_flush_to_log(serial_s_tty_n_1, log_buf, log_bufpos);
		/*We can check the key word within host log here*/
		/*
		 * Host log string match function will start when str_match_enable was set
		 *
		 */
		if(!access(BIOS_UPGRADE_STARTED,F_OK))
		{
			g_is_console.match_str.str_match_enable = 1;
			console_m_debug("Found the file %s ,str match started \n",BIOS_UPGRADE_STARTED);
			system("rm "BIOS_UPGRADE_STARTED);

		}
		if(g_is_console.match_str.str_match_enable)
		{
			if( g_is_console.match_str.bios_type == BIOS_TYPE_UNKNOW)
			{
				/*setting the bios type and init some value*/
				g_is_console.match_str.bios_type = BIOS_TYPE_UBMC;
				g_is_console.match_str.bios_state = BIOS_BOOT_START;
				g_is_console.match_str.match_str_pos = 0;

			}
			if(g_is_console.match_str.bios_type == BIOS_TYPE_UBMC)
			{
				if(g_is_console.match_str.bios_state == BIOS_BOOT_START)
				{
					is_matched = console_manager_match_str(BIOS_BOOT_LOG1,log_buf,buf_len);
					if(is_matched)
					{
						g_is_console.match_str.bios_state = BIOS_BOOT_STATE_1;
						console_m_debug("match the str %s \n",BIOS_BOOT_LOG1);
					}
				}
				if(g_is_console.match_str.bios_state == BIOS_BOOT_STATE_1)
				{
					is_matched = console_manager_match_str(BIOS_BOOT_LOG2,log_buf,buf_len);
					if(is_matched)
					{
						g_is_console.match_str.bios_state = BIOS_BOOT_STATE_2;
						console_m_debug("match the str %s \n",BIOS_BOOT_LOG2);
					}
				}
				if(g_is_console.match_str.bios_state == BIOS_BOOT_STATE_2)
				{
					is_matched = console_manager_match_str(BIOS_BOOT_LOG3,log_buf,buf_len);
					if(is_matched)
					{
						wlen = 1;
						console_manager_flush_to_serial_source(serial_s_tty_n_1, "\r", &wlen);	/*answer a "Enter" key when receive all information*/
						g_is_console.match_str.bios_state = BIOS_BOOT_STATE_3;
						console_m_debug("match the str %s \n",BIOS_BOOT_LOG3);
					}
				}
				if(g_is_console.match_str.bios_state == BIOS_BOOT_STATE_3)
				{
					is_matched = console_manager_match_str(BIOS_BOOT_LOG4,log_buf,buf_len);
					if(is_matched)
					{
						console_m_debug("match all str \n");
						g_is_console.match_str.bios_state = BIOS_BOOT_END;
					}
				}
				if(g_is_console.match_str.bios_state == BIOS_BOOT_END)
				{
					/*Todo :BIOS startup has been finished, do some deinit*/
					g_is_console.match_str.bios_type = BIOS_TYPE_UNKNOW;
					g_is_console.match_str.str_match_enable = 0;
					g_is_console.match_str.match_str_pos = 0;
					if(!access(BIOS_UPGRADE_DONE,F_OK))
					{
						console_m_err("%s has been exist ,please check %s\n",BIOS_UPGRADE_DONE);
					}
					system("touch "BIOS_UPGRADE_DONE);
				}
			}
			/*Handle the thing when BIOS upgrade time out*/
			if(!access(BIOS_UPGRADE_TIMEOUT,F_OK))
			{
				console_m_err("BIOS upgrade time out ,state is in %d ,disable string match \n",g_is_console.match_str.bios_state);
				g_is_console.match_str.bios_type = BIOS_TYPE_UNKNOW;
				g_is_console.match_str.str_match_enable = 0;
				g_is_console.match_str.match_str_pos = 0;
				system("rm "BIOS_UPGRADE_TIMEOUT);
			}

		}
	}

	return 0;
}

int32_t console_manager_flush_client_socket_to_serial_source(int32_t serial_dev_no)
{
	uint *client_socket_bufpos;
	char * client_socket_buf;

	client_socket_bufpos = &(g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos);
	client_socket_buf = g_con_mgr_serial_dev[serial_dev_no].client_socket_buf;

	console_manager_flush_to_serial_source(serial_dev_no, client_socket_buf, client_socket_bufpos);

	return 0;
}

int32_t console_manager_flush_to_client_socket_tips_msg(int32_t serial_dev_no, int32_t to_client_fd,
		int32_t tips_msg_pos)
{
	int32_t todo, done;

	todo = strlen(console_manager_msg[tips_msg_pos]);
	console_m_debug("pos[%d] \n", todo);
	while (todo > 0)
	{
		done = write(to_client_fd, console_manager_msg[tips_msg_pos], todo);
		if (done > 0)
		{
			todo -= done;
		}

		if (done < 0 && errno != EINTR)
			break;
	}
	console_m_debug("End pos[%d] \n", todo);

	close(to_client_fd);

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_kick_out_client_socket(int32_t serial_dev_no, int32_t msg_type)
{
	int32_t client_socket_fd;

	client_socket_fd = g_con_mgr_serial_dev[serial_dev_no].clients[to_client_pollfd].fd;
	console_manager_flush_to_client_socket_tips_msg(serial_dev_no, client_socket_fd, msg_type);

	g_con_mgr_serial_dev[serial_dev_no].is_establish_con = 0;

	/*
	 * initiative to exit
	 */
	clear_bit_status(g_con_mgr_serial_dev[serial_dev_no].is_kick_con, BIT_IS_KICK_CON_SELF_EXIT_STATUS);

	console_m_notice("console_manager socket connection is disconneted \n");

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_capture_special_chars_cmd_type(char *buf, int32_t buf_size)
{
	int32_t i;

	if (buf)
	{
		for (i = 0; i < buf_size; i++)
		{
			if (buf[i] == 0x18)
			{
				//ctrl + X ascii code
				console_m_debug("capture ctrl + X signal \n");
				return CMD_CTRL_X;
			}
			else if (buf[i] == 0x1d)
			{
				//ctrl + ] ascii code
				console_m_debug("capture ctrl + ] signal \n");
				return CMD_CTRL_EXIT;
			}

		}
	}

	return CMD_NONE;
}

int32_t console_manager_set_global_status(int32_t serial_dev_no, int32_t cmd_type,int8_t flag)
{
	switch (cmd_type)
	{
	case CMD_CTRL_X:

		if(flag == 1)
		{
			//ignor this signal
			//set_bit_status(g_con_mgr_serial_dev[serial_dev_no].is_kick_con, BIT_IS_KICK_CON_SELF_EXIT_STATUS);
			break;
		}
		if (current_serial_source == serial_s_tty_n_1)
		{
			current_serial_source = serial_s_uart_sim;
		}
		else
		{
			current_serial_source = serial_s_tty_n_1;
		}

		console_m_notice("console_manager set serial source switched status\n");

		break;
	case CMD_CTRL_EXIT:
		if(flag == 1 )
		{
			set_bit_status(g_con_mgr_serial_dev[serial_dev_no].is_kick_con, BIT_IS_KICK_CON_SELF_EXIT_STATUS);
			console_m_notice("console_manager set self exit status\n");
		}
		break;
	default:
		break;
	}
	return 0;
}

int32_t console_manager_read_serial_out_buf(int32_t serial_dev_no, int32_t *serial_out_bytes_read, int32_t *res)
{
	int32_t i, res_type;
	int32_t serial_out_fd;
	char * serial_out_buf;
	int32_t serial_out_bufroom;
	int32_t buf_offset;

	serial_out_fd = g_con_mgr_serial_dev[serial_dev_no].clients[serial_output_pollfd].fd;
	serial_out_buf = g_con_mgr_serial_dev[serial_dev_no].serial_out_buf;

	buf_offset = g_con_mgr_serial_dev[serial_dev_no].serial_out_bufpos;
	serial_out_bufroom = IS_CON_MGR_TTY_BUF_SIZE - buf_offset;

	i = read(serial_out_fd, serial_out_buf + buf_offset, serial_out_bufroom);
	//printf("read out buf :%s ,i is %d \n",serial_out_buf + buf_offset,i);
	if (i > 0)
	{
		con_mgr_dump(serial_out_bytes_read+buf_offset, i, serial_dev_no);

		g_con_mgr_serial_dev[serial_dev_no].serial_out_bufpos += i;
		if (serial_out_bytes_read)
			*serial_out_bytes_read = i;
		set_bit_status(*res, BIT_READ_FROM_SERIAL_OUT_STATUS);
		console_m_debug("read[%d] pos[%d] \n", i, g_con_mgr_serial_dev[serial_dev_no].serial_out_bufpos);

		/*check input chars */
		res_type = console_manager_capture_special_chars_cmd_type(serial_out_buf,
				g_con_mgr_serial_dev[serial_dev_no].serial_out_bufpos);
		console_manager_set_global_status(serial_dev_no, res_type,0);
	}

	return 0;

}

int32_t console_manager_read_serial_source(int32_t serial_dev_no, int32_t *serial_in_bytes_read, int32_t *res)
{
	int32_t i, len;
	int32_t serial_in_fd, serial_in_bufroom;
	int32_t serial_in_socket_fd, serial_in_socket_bufroom;
	char * serial_in_buf;
	char * serial_in_socket_buf;
	char * log_buf;
	int32_t serial_in_buf_offset, serial_in_socket_buf_offset, log_buf_offset;
	int32_t log_buf_room;

	serial_in_fd = g_con_mgr_serial_dev[serial_dev_no].clients[serial_source_in_pollfd].fd;
	serial_in_buf = g_con_mgr_serial_dev[serial_dev_no].serial_in_buf;

	serial_in_buf_offset = g_con_mgr_serial_dev[serial_dev_no].serial_in_bufpos;
	serial_in_bufroom = IS_CON_MGR_TTY_BUF_SIZE - serial_in_buf_offset;

	serial_in_socket_buf = g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_buf;
	serial_in_socket_buf_offset = g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_bufpos;
	serial_in_socket_bufroom = IS_CON_MGR_TTY_BUF_SIZE - serial_in_socket_buf_offset;

	log_buf = g_con_mgr_serial_dev[serial_dev_no].log_buf;
	log_buf_offset = g_con_mgr_serial_dev[serial_dev_no].log_bufpos;
	log_buf_room = IS_CON_MGR_TTY_BUF_SIZE - g_con_mgr_serial_dev[serial_dev_no].log_bufpos;

	is_config_console *p_config;
	p_config = g_is_console.p_config;

	len = min(serial_in_bufroom, serial_in_socket_bufroom);
	//printf("serial_in_bufroom is %d ,serial_in_socket_bufroom is %d \n",serial_in_bufroom,serial_in_socket_bufroom);
	i = read(serial_in_fd, serial_in_buf + serial_in_buf_offset, len);
	//printf("console_manager_read_serial_source :read :%s,len is %d,i is %d ,serial_in_fd is %d \n",serial_in_buf + serial_in_buf_offset,len,i,serial_in_fd);
	if (i > 0)
	{
		/*
		 * if socket connect,then copy serial source data to serial_in_socket_buf
		 * besides,the other endpoint socket client of ipc may be closed unexpectly
		 */
		con_mgr_dump(serial_in_buf + serial_in_buf_offset, i, serial_dev_no);

		if (g_con_mgr_serial_dev[serial_dev_no].is_establish_con)
		{
			memcpy(serial_in_socket_buf + serial_in_socket_buf_offset, serial_in_buf + serial_in_buf_offset, i);
			g_con_mgr_serial_dev[serial_dev_no].serial_in_socket_bufpos += i;
		}
		else
		{
			;
		}

		//check for bios boiot waiting for ubmc to be ready
		if(serial_dev_no == serial_s_tty_n_1)
		{
			console_manager_chk_bios_boot_wait(serial_in_buf + serial_in_buf_offset, i);
		}

		if (serial_dev_no == serial_s_tty_n_1 && p_config->is_log_to_file)
		{
			//printf("write log :i is %d ,log_buf_room is %d \n",i,log_buf_room);
			//printf("log buf is %s \n",serial_in_buf + serial_in_buf_offset);
			memcpy(log_buf + log_buf_offset, serial_in_buf + serial_in_buf_offset, min(i, log_buf_room));
			g_con_mgr_serial_dev[serial_dev_no].log_bufpos += min(i, log_buf_room);
			if (i > log_buf_room)
			{

				console_m_err("%s log will lost", serial_dev_no == serial_s_tty_n_1 ? "HOST" : "UBMC");

			}
		}
		if(serial_dev_no == current_serial_source)
		{
			g_con_mgr_serial_dev[serial_dev_no].serial_in_bufpos += i;
		}
		else
		{
			;
		}

		if (serial_in_bytes_read)
			*serial_in_bytes_read = i;

		console_m_debug("read[%d] pos[%d] \n", i, g_con_mgr_serial_dev[serial_dev_no].serial_in_bufpos);
		set_bit_status(*res, BIT_READ_FROM_SERIAL_SOURCE_STATUS);
	}

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_server_socket_accept(int32_t serial_dev_no, int32_t *to_client_fd, int32_t *res)
{
	int32_t server_socket_fd, client_socket_fd;
	struct sockaddr_un to_client_addr;

	socklen_t fromlen;

	/*
	 *warnning :!!!!!!
	 *fromlen must be initialized with positive value,
	 *otherwise accept api may throw invalid argument error when
	 * variable is initialized with negative value randomly on stack
	 *
	 */
	fromlen = sizeof(struct sockaddr);

	server_socket_fd = g_con_mgr_serial_dev[serial_dev_no].clients[socket_pollfd].fd;
	client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &to_client_addr, &fromlen);
	console_m_debug("accept\n");
	if (client_socket_fd > 0)
	{
		*to_client_fd = client_socket_fd;

		set_bit_status(*res, BIT_SOCKET_ACCEPT_STATUS);
		console_m_notice("console_manager socket accept sucessfully\n");
	}
	else
	{
		console_m_err("console_manager socket accept fail [%s]", strerror(errno));
	}

	return IS_CON_MGR_ERR_NULL;

}

int32_t console_manager_read_client_socket_buf(int32_t serial_dev_no, int32_t *client_socket_bytes_read, int32_t *res)
{

	char * to_client_socket_buf;
	int32_t to_client_socket_bufroom, to_client_socket_fd, i;
	int32_t res_type;
	int32_t buf_offset;

	if (g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos > IS_CON_MGR_TTY_BUF_SIZE)
		g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos = 0;

	to_client_socket_fd = g_con_mgr_serial_dev[serial_dev_no].clients[to_client_pollfd].fd;
	to_client_socket_buf = g_con_mgr_serial_dev[serial_dev_no].client_socket_buf;

	buf_offset = g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos;
	to_client_socket_bufroom = IS_CON_MGR_TTY_BUF_SIZE - buf_offset;

	i = read(to_client_socket_fd, to_client_socket_buf + buf_offset, to_client_socket_bufroom);
	if (i > 0)
	{
		g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos += i;
		if (client_socket_bytes_read)
			*client_socket_bytes_read = i;

		set_bit_status(*res, BIT_READ_FROM_CLIENT_SOCKET_STATUS);
		console_m_debug("read %d bytes\n", i);

		/*check input chars */
		res_type = console_manager_capture_special_chars_cmd_type(to_client_socket_buf,
				g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos);
		console_m_debug("debug :read res_type is %d bytes\n", res);
		console_manager_set_global_status(serial_dev_no, res_type,1);
	}

	if (i == 0)
	{

		/*
		 *!!!!!!
		 *note: if the socket client  is not available,
		 *    the value of i will be zero in the case of calling poll function
		 *
		 */
		g_con_mgr_serial_dev[serial_dev_no].is_establish_con = 0;
		close(to_client_socket_fd);
		console_m_warn("the socket client is not available ,set is_establish_con off\n");

	}

	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_set_client_socket_kick_out_status(int32_t *res)
{

	set_bit_status(*res, BIT_KICK_OUT_CLIENT_SOCKET_STATUS);
	console_m_debug("set client socket kick out status \n");

	return 0;
}

void console_manager_set_cli_control_status(int32_t serial_dev_no, struct mesg_buffer *msg_buf)
{
	if (msg_buf)
	{
		switch (msg_buf->mesg_type)
		{
		case CLI_CMD_TYPE_KICK_OUT_SOCKET:

			console_m_notice("console_manager set socket existing connection kicked out status get signal\n");
			if (serial_dev_no == serial_s_tty_n_1 && g_con_mgr_serial_dev[serial_dev_no].is_establish_con)
			{
				/*
				 *Passive exit
				 */
				set_bit_status(g_con_mgr_serial_dev[serial_dev_no].is_kick_con, BIT_IS_KICK_CON_FORCE_EXIT_STATUS);
				console_m_notice("console_manager set socket existing connection kicked out status\n");
			}

			break;
		default:
			break;
		}

	}
}

int32_t console_manager_do_cli_cmd_to_set_status(int32_t serial_dev_no)
{
	int32_t msgid;
	int32_t len = 0, i;
	struct mesg_buffer *msg_buf;

	/*
	 *now only host serial is allowed message queue  communications
	 */
	if (serial_dev_no == serial_s_tty_n_1)
	{
		msgid = g_con_mgr_serial_dev[serial_dev_no].msgid;
		msg_buf = g_con_mgr_serial_dev[serial_dev_no].queue_msg;

		for (i = CLI_CMD_TYPE_KICK_OUT_SOCKET; i < CLI_CMD_TYPE_MAX; i++)
		{
			/*
			 * msgrcv to receive message
			 */
			len = msgrcv(msgid, msg_buf, sizeof(struct mesg_buffer), i,
			IPC_NOWAIT);
			if (len > 0)
			{
				/*
				 * there is a message
				 */
				console_manager_set_cli_control_status(serial_dev_no, msg_buf);

			}

		}

	}
	else
	{

		/*
		 * Under normal circumstances,
		 *the value of serial_dev_no and shm_p[shm_pos_serial_source] should be equal
		 * unless the serial socket client change it
		 */
		if (shm_p[shm_pos_serial_source] == serial_s_tty_n_1)
		{
			console_manager_set_global_status(serial_dev_no, CMD_CTRL_X,0);
			console_m_notice("serial over lan switched serial source");
		}
	}

	return 0;
}
struct pollfd g_all_clients[pollfd_max * 2];
/**
 * Just map the g_con_mgr_serial_dev[0] and g_con_mgr_serial_dev[1]
 * pollfd to the g_all_clients which hold all pollfd needed to poll
 *
 */
int32_t console_manager_poll_all(struct serial_source* source,int32_t tmout,int32_t *offset)
{

	int32_t data, pullfd_num_0,pullfd_num_1;
	int32_t is_establish_con;
	char i = 0;
	is_establish_con = g_con_mgr_serial_dev[0].is_establish_con;
/*	for(i = 0;i < 4;i ++)
	{
		printf("client0 %d is %d  \n",i,g_con_mgr_serial_dev[0].clients[i]);
		printf("client1 %d is %d  \n",i,g_con_mgr_serial_dev[1].clients[i]);
	}*/
	if (!is_establish_con)
	{
		pullfd_num_0 = pollfd_max - 1;
	}
	else
	{
		pullfd_num_0 = pollfd_max;
	}

	for(i = 0;i <pollfd_max ; i++)
	{
		g_all_clients[i] = g_con_mgr_serial_dev[0].clients[i];
		//printf("client %d is %d  \n",i,g_all_clients[i]);
	}
	*offset = pullfd_num_0;
	is_establish_con = g_con_mgr_serial_dev[1].is_establish_con;
	if (!is_establish_con)
	{
		pullfd_num_1 = pollfd_max - 1;
	}
	else
	{
		pullfd_num_1 = pollfd_max;
	}
	for(i = pollfd_max;i <pollfd_max*2 ; i++)
	{
		g_all_clients[i] = g_con_mgr_serial_dev[1].clients[i-pollfd_max];
		//printf("client %d is %d  \n",i,g_all_clients[i]);
	}
	data = poll(g_all_clients, pollfd_max*2, tmout);
	return data;
}

int32_t console_manager_check_io(int32_t serial_dev_no, int32_t tmout, int32_t *serial_in_bytes_read,
		int32_t *serial_out_bytes_read, int32_t *to_client_fd, int32_t *client_socket_bytes_read)
{
	int32_t n = 0;
	int32_t data, pullfd_num;
	int32_t is_establish_con;
	int32_t offset;
	int32_t serial_out_fd;

	data = console_manager_poll_all(g_con_mgr_serial_dev,tmout,&offset);
	if (data > 0)
	{
/*		serial_s_uart_sim do not support socket client
 * 		if (g_all_clients[2].revents & POLLIN)
		{
			console_manager_server_socket_accept(serial_dev_no, to_client_fd, &n);
			//printf("ubmc socket_pollfd serial_dev_no is %d n is %d \n",serial_dev_no,n);
		}
		if (g_all_clients[3].revents & POLLIN)
		{
			console_manager_read_client_socket_buf(serial_dev_no, client_socket_bytes_read, &n);
			//printf("ubmc to_client_pollfd serial_dev_no is %d n is %d \n",serial_dev_no,n);
		}
*/
		//poll serial input from serial_s_uart_sim
		if (g_all_clients[0].revents & POLLIN)
		{
			console_manager_read_serial_source(serial_s_uart_sim, serial_in_bytes_read, &n);
			//printf("ubmc serial_source_in_pollfd serial_dev_no is %d n is %d \n",serial_dev_no,n);
		}

/*			if (g_all_clients[1].revents & POLLIN)
		{

			console_manager_read_serial_out_buf(serial_dev_no, serial_out_bytes_read, &n);
			printf("ubmc serial_output_pollfd serial_dev_no is %d n is %d \n",serial_dev_no,n);
		}
*/

		//poll socket accept ,only for serial_s_tty_n_1
		if (g_all_clients[6].revents & POLLIN)
		{
			console_manager_server_socket_accept(serial_s_tty_n_1, to_client_fd, &n);
			//printf("host socket_pollfd serial_dev_no is %d n is %d \n",serial_dev_no,n);
		}
		//poll socket accept ,only for serial_s_tty_n_1
		if (g_all_clients[7].revents & POLLIN)
		{
			console_manager_read_client_socket_buf(serial_s_tty_n_1, client_socket_bytes_read, &n);
			//printf("host to_client_pollfd serial_dev_no is %d n is %d\n",serial_dev_no,n);
		}
		//poll serial input from serial_s_tty_n_1
		if (g_all_clients[4].revents & POLLIN)
		{
			int serial_in_fd;
			int i;
			char buf[1024] = {0};
			serial_in_fd = g_con_mgr_serial_dev[serial_s_tty_n_1].clients[serial_source_in_pollfd].fd;
			//i = read(serial_in_fd, buf, 2);
			//printf("buf %s i is %d serial_in_fd is %d ,pollfd is %d \n",buf,i,serial_in_fd,g_all_clients[4].fd);
			console_manager_read_serial_source(serial_s_tty_n_1, serial_in_bytes_read, &n);
		//	printf("host  serial_source_in_pollfd serial_dev_no is %d n is %d serial_in_fd is %d \n",serial_dev_no,n,serial_in_fd);
		}
		//the g_all_clients[1] is same to g_all_clients[5]
		//if (g_all_clients[1].revents & POLLIN)
		if (g_all_clients[5].revents & POLLIN)
		{
			console_manager_read_serial_out_buf(serial_dev_no, serial_out_bytes_read, &n);
			//printf("host serial_output_pollfd serial_dev_no is %d n is %d \n",serial_dev_no,n);
		}
	}
	else
	{
		//printf("poll time out \n");
	}

#if 0
	if(data > 0)
	{
		/*only host serial ttyN1 have socket function*/
		if (current_serial_source == serial_s_tty_n_1 )
		{
			if (g_con_mgr_serial_dev[serial_dev_no].clients[socket_pollfd].revents & POLLIN)
			{
				console_manager_server_socket_accept(serial_dev_no, to_client_fd, &n);
			}
			if (is_establish_con && (g_con_mgr_serial_dev[serial_dev_no].clients[to_client_pollfd].revents & POLLIN))
			{
				console_manager_read_client_socket_buf(serial_dev_no, client_socket_bytes_read, &n);
			}

		}

		if ((g_con_mgr_serial_dev[serial_dev_no].clients[serial_output_pollfd].revents & POLLIN))
		{
			console_manager_read_serial_out_buf(serial_dev_no, serial_out_bytes_read, &n);
		}

		if (g_con_mgr_serial_dev[serial_dev_no].clients[serial_source_in_pollfd].revents & POLLIN)
		{
			console_manager_read_serial_source(serial_dev_no, serial_in_bytes_read, &n);
		}
	}
#endif


	if (current_serial_source == serial_s_tty_n_1 && is_establish_con
			&& (get_bit_status(g_con_mgr_serial_dev[serial_dev_no].is_kick_con, BIT_IS_KICK_CON_SELF_EXIT_STATUS)))
	{
		//printf("do console_manager_set_client_socket_kick_out_status \n");
		console_manager_set_client_socket_kick_out_status(&n);
	}
	//printf("n is %d \n",n);
	return n;
}

int32_t console_manager_add_client_socket_to_poll(int32_t serial_dev_no, int32_t client_fd)
{
	memset(g_con_mgr_serial_dev[serial_dev_no].client_socket_buf, 0, IS_CON_MGR_TTY_BUF_SIZE);

	g_con_mgr_serial_dev[serial_dev_no].client_socket_bufpos = 0;
	g_con_mgr_serial_dev[serial_dev_no].clients[to_client_pollfd].fd = client_fd;
	g_con_mgr_serial_dev[serial_dev_no].clients[to_client_pollfd].events = POLLIN;

	console_m_notice("console_manager establish socket connection\n");
	return 0;
}

void console_manager_output_switching_serial_source_msg_tips(int32_t serial_dev_no)
{
	char *buf;
	int32_t len;

	if (serial_dev_no
			== serial_s_uart_sim&& get_bit_status(g_con_mgr_serial_dev[serial_dev_no].console_output_msg_tips_st,serial_s_uart_sim))
	{
		buf = console_manager_msg[message_switching_uart_sim_serial_source_tips];
		len = strlen(buf);
		console_m_debug("send switched message tips to UBMC:%s \n", buf);
		console_manager_flush_to_serial_out(serial_dev_no, buf, &len);
		clear_bit_status(g_con_mgr_serial_dev[serial_dev_no].console_output_msg_tips_st, serial_s_uart_sim);

	}
	else if (serial_dev_no
			== serial_s_tty_n_1&& get_bit_status(g_con_mgr_serial_dev[serial_dev_no].console_output_msg_tips_st,serial_s_tty_n_1))
	{
		buf = console_manager_msg[message_switching_tty_n_1_serial_source_tips];
		len = strlen(buf);
		console_m_debug("send switched message tips to HOST:%s \n", buf);
		console_manager_flush_to_serial_out(serial_dev_no, buf, &len);
		if (g_con_mgr_serial_dev[serial_dev_no].is_establish_con)
		{
			console_manager_flush_to_socket(serial_dev_no, buf, &len);
		}
		clear_bit_status(g_con_mgr_serial_dev[serial_dev_no].console_output_msg_tips_st, serial_s_tty_n_1);

	}

}

void console_manager_do_switch_serial_source(int32_t * old_serial_source, int32_t *serial_source)
{

	if ((g_con_mgr_serial_dev[*old_serial_source].is_establish_con) && (*old_serial_source == serial_s_tty_n_1)
			&& (*serial_source == serial_s_uart_sim))
	{
		//console_manager_kick_out_client_socket(*old_serial_source, message_socket_client_switch_serial_source_tips);
	}

	*old_serial_source = *serial_source;

	if (*old_serial_source == serial_s_uart_sim)
	{

		set_bit_status(g_con_mgr_serial_dev[*old_serial_source].console_output_msg_tips_st, serial_s_uart_sim);

	}
	else if (*old_serial_source == serial_s_tty_n_1)
	{

		set_bit_status(g_con_mgr_serial_dev[*old_serial_source].console_output_msg_tips_st, serial_s_tty_n_1);

	}

	shm_p[shm_pos_serial_source] = *serial_source;

	//write the Share Memory to clear switched serial_source status for sol
	if (shm_p[shm_pos_status] == shm_op_st_processing)
		shm_p[shm_pos_status] = shm_op_st_confirm;
	console_m_notice("console_manager serial source switched : now serial source[%s]\n",
			*serial_source == serial_s_tty_n_1 ? "HOST" : "UBMC");
}

int32_t console_manager_socket_init(int32_t *server_socket_fd)
{
	struct sockaddr_un server_socket_addr;
	register int32_t socket_len;
	int32_t res = 0;

	if ((*server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		console_m_err("console_manager error server: socket [%d]", *server_socket_fd);
		return IS_CON_MGR_ERR_SERVER_SOCKET;
	}

	/*
	 * Create the address we will be binding to.
	 */
	server_socket_addr.sun_family = AF_UNIX;
	strcpy(server_socket_addr.sun_path, ADDRESS);

	/*
	 * Try to bind the address to the socket.  We
	 * unlink the name first so that the bind won't
	 * fail.
	 *
	 * The third argument indicates the "length" of
	 * the structure, not just the length of the
	 * socket name.
	 */
	unlink(ADDRESS);
	socket_len = sizeof(server_socket_addr.sun_family) + strlen(server_socket_addr.sun_path);

	if ((res = bind(*server_socket_fd, (struct sockaddr *) &server_socket_addr, socket_len)) < 0)
	{
		console_m_err("console_manager error server: bind [%d]", res);
		return IS_CON_MGR_ERR_SERVER_BIND;
	}
	if (chmod(ADDRESS, 0666) < 0)
				console_m_warn("console_manager cannot change file(%s) permission to 0644 \n",ADDRESS);
	/*
	 * Listen on the socket.
	 */
	if ((res = listen(*server_socket_fd, 5)) < 0)
	{
		console_m_err("console_manager error server: listen [%d]", res);
		return IS_CON_MGR_ERR_SERVER_LISTEN;
	}

	console_m_info("console_manager init socket sucessfully and listen on the socket\n");
	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_serial_source_open(int32_t * serial_s_fd1, int32_t *serial_s_fd2, int32_t *serial_out_fd,
		int32_t *log_fd)
{
	*serial_s_fd1 = open(CONSOLE_MANAGER_SERIAL_SOURCE_1,
	O_RDWR | O_NOCTTY | O_SYNC);
	if (*serial_s_fd1 < 0)
	{
		console_m_err("console_manager error opening %s: %s\n", CONSOLE_MANAGER_SERIAL_SOURCE_1, strerror (errno));
		return IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED;
	}

	*serial_s_fd2 = open(console_manager_serial_source_2,
	O_RDWR | O_NOCTTY | O_SYNC,S_IRWXU|S_IRWXO);
	if (*serial_s_fd2 < 0)
	{
		console_m_err("console_manager error opening %s: %s\n", console_manager_serial_source_2, strerror (errno));
		return IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED;
	}
	if (chmod(console_manager_serial_source_2, 0666) < 0)
				console_m_warn("console_manager cannot change file(%s) permission to 0644 \n",
						console_manager_serial_source_2);
	*serial_out_fd = open(console_manager_serial_output,
	O_RDWR | O_NOCTTY | O_SYNC);
	if (*serial_out_fd < 0)
	{
		console_m_err("console_manager error opening %s: %s\n", console_manager_serial_output, strerror (errno));
		return IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED;
	}

	*log_fd = open(CONSOLE_MANAGER_SERIAL_HOST_LOG,
	O_CREAT | O_RDWR | O_APPEND);
	if (*log_fd < 0)
	{
		console_m_err("console_manager error opening %s: %s\n", CONSOLE_MANAGER_SERIAL_HOST_LOG, strerror (errno));
		return IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED;
	}
	else
	{
		if (chmod(CONSOLE_MANAGER_SERIAL_HOST_LOG, 0644) < 0)
			console_m_warn("console_manager cannot change file(%s) permission to 0644 \n",
					CONSOLE_MANAGER_SERIAL_HOST_LOG);
	}

	console_m_info(
			"console_manager open serial_source_1 : %s serial_source_2 :%s serial output :%s host_serial_log :%s\n",
			CONSOLE_MANAGER_SERIAL_SOURCE_1, console_manager_serial_source_2, console_manager_serial_output,
			CONSOLE_MANAGER_SERIAL_HOST_LOG);

	return IS_CON_MGR_ERR_NULL;

}

int32_t console_manager_do_socket_accept(int32_t serial_source, int32_t to_client_socket_fd)
{
	int32_t client_socket_fd;

	if (!g_con_mgr_serial_dev[serial_source].is_establish_con)
	{

		g_con_mgr_serial_dev[serial_source].is_establish_con++;
		console_manager_add_client_socket_to_poll(serial_source, to_client_socket_fd);

	}
	else
	{

		if (get_bit_status(g_con_mgr_serial_dev[serial_source].is_kick_con, BIT_IS_KICK_CON_FORCE_EXIT_STATUS))
		{
			/*
			 *the new socket connection will replace old one when socket client
			 *login with parameter -f
			 */

			client_socket_fd = g_con_mgr_serial_dev[serial_source].clients[to_client_pollfd].fd;

			//close and send message to socket connection
			console_manager_flush_to_client_socket_tips_msg(serial_source, client_socket_fd,
					message_socket_client_kick_out_tips);

			console_manager_add_client_socket_to_poll(serial_source, to_client_socket_fd);

			clear_bit_status(g_con_mgr_serial_dev[serial_source].is_kick_con, BIT_IS_KICK_CON_FORCE_EXIT_STATUS);
			console_m_debug("[pre]:%d [now]:%d \n", client_socket_fd, to_client_socket_fd);

		}
		else
		{

			client_socket_fd = to_client_socket_fd;
			//close and send message to socket connection
			console_manager_flush_to_client_socket_tips_msg(serial_source, client_socket_fd,
					message_socket_client_exist_tips);

		}

	}

	return IS_CON_MGR_ERR_NULL;
}

static void console_manager_signal_handler(int32_t sig)
{
	int32_t serial_dev_no;
	int32_t client_socket_fd;

	console_m_warn("SIGPIPE signal catch");

	/*
	 *only host serial source supports socket
	 */
	serial_dev_no = serial_s_tty_n_1;

	g_con_mgr_serial_dev[serial_dev_no].is_establish_con = 0;

	client_socket_fd = g_con_mgr_serial_dev[serial_dev_no].clients[to_client_pollfd].fd;

	close(client_socket_fd);

	/*
	 * clear socket buffer
	 */
	g_con_mgr_serial_dev[serial_dev_no].serial_in_bufpos = 0;

}

int32_t console_manager_do_main(int32_t serial_source)
{
	int32_t serial_in_len = 0, serial_out_len = 0, socket_read_len = 0;
	int32_t to_client_socket_fd;
	/*
	 *status of io
	 */
	int32_t io_st;

	io_st = console_manager_check_io(serial_source, 100, &serial_in_len, &serial_out_len, &to_client_socket_fd,
			&socket_read_len);
	console_manager_do_cli_cmd_to_set_status(serial_source);
	console_manager_mgmtd_config_status_check(serial_source);

	if(serial_source == 0)
	{
		//when serial_source on ubmc
		uint *serial_in_socket_bufpos;
		char * serial_in_socket_buf;
		console_manager_flush_client_socket_to_serial_source(serial_s_tty_n_1);
		console_manager_flush_to_input(serial_s_tty_n_1);
		//console_manager_flush_to_output(1);
		serial_in_socket_bufpos = &(g_con_mgr_serial_dev[serial_s_tty_n_1].serial_in_socket_bufpos);
		serial_in_socket_buf = g_con_mgr_serial_dev[serial_s_tty_n_1].serial_in_socket_buf;
		//console_m_notice("console_manager_flush_to_socket %s ",serial_in_socket_buf);
		console_manager_flush_to_socket(serial_s_tty_n_1, serial_in_socket_buf, serial_in_socket_bufpos);
	}

	if (get_bit_status(io_st, BIT_READ_FROM_SERIAL_OUT_STATUS))
	{
		if (serial_out_len > 0)
		{
			console_manager_flush_to_input(serial_source);
		}
	}

	if (get_bit_status(io_st, BIT_READ_FROM_SERIAL_SOURCE_STATUS))
	{
		if (serial_in_len > 0)
		{
			console_manager_flush_to_output(serial_source);
		}
	}

	if (get_bit_status(io_st, BIT_SOCKET_ACCEPT_STATUS))
	{
		if (to_client_socket_fd > 0)
		{
			// only serial_s_tty_n_1 have socket client
			console_manager_do_socket_accept(serial_s_tty_n_1, to_client_socket_fd);
		}
	}

	if (get_bit_status(io_st, BIT_READ_FROM_CLIENT_SOCKET_STATUS))
	{
		if (socket_read_len > 0)
		{

			console_manager_flush_client_socket_to_serial_source(serial_source);
		}
	}

	if (get_bit_status(io_st, BIT_KICK_OUT_CLIENT_SOCKET_STATUS))
	{

		console_manager_kick_out_client_socket(serial_source, message_socket_client_kick_out_tips);
	}
	return IS_CON_MGR_ERR_NULL;
}

int32_t console_manager_shm_init(int32_t **shm)
{
	int32_t shmid;
	int32_t *tmp_ptr;

	/*
	 * Create the segment.
	 */
	if ((shmid = shmget(shm_key, SHMSZ, IPC_CREAT | 0666)) < 0)
	{

		return IS_CON_MGR_ERR_FAIL_CREATE_SHM;

	}

	/*
	 * Now we attach the segment to our data space.
	 */
	if ((*shm = shmat(shmid, NULL, 0)) == (int32_t *) -1)
	{

		return IS_CON_MGR_ERR_FAIL_CREATE_SHM;

	}

	tmp_ptr = *shm;
	tmp_ptr[shm_pos_status] = shm_op_st_null;

	return IS_CON_MGR_ERR_NULL;
}

extern is_config_console g_is_config_console;
static void set_dft_console()
{
	g_is_console.p_config = &g_is_config_console;
	is_config_console *p_config = g_is_console.p_config;
	p_config->com_hwflowctrl = true;
	p_config->com_swflowctrl = false;
	p_config->is_log_to_file = false;
	p_config->com_speed = 115200;
	p_config->com_data = 8;
	p_config->com_stopbits = 1;

	g_is_console.is_initalized = true;
	g_is_console.is_status_changed = false;
}

void usage(char * prog)
{
	fprintf(stderr,
			"usage: %s [-H <host tty name>] [-O <output tty name>]\n \
            -h : help \n \
            -H : host tty name  \n \
			-O : output tty name \n \
			For example: \n \
			%s -O /dev/ttyMV0 -H /dev/ttyMV1 \n \
			%s -O /dev/ttyS0 -H /dev/ttyS1 \n",prog,prog,prog);
}

int32_t main(int32_t argc, char *argv[])
{
	int32_t server_socket_fd;

	int32_t serial_source;
	int32_t serial_source_fd1;
	int32_t serial_source_fd2;
	int32_t serial_out_fd;
	int32_t host_log_fd;
	int32_t host_serial_fd;
#ifdef DEBUG
	int32_t ubmc_log_fd;
#endif
	int32_t res;
	int32_t lock_file;
	int32_t lock_res;


	char * prog;
	int opt;
	int ret;
	prog = argv[0];
	while ((opt = getopt(argc, argv, "H:O:h?")) != -1)
	{
		switch (opt)
		{
		//The source tty name
		case 'H':
			if(optarg != NULL)
			{
				//fan_setting.interv = atoi(optarg);
				strcpy(console_manager_serial_source_2,optarg);

				//console_m_err("console_manager arg:H %s\n",optarg);
			}
			break;
			//printf("t case optarg is %s interv is %d \n",optarg,fan_setting.interv);
		//The OUTPUT tty name
		case 'O':
			if(optarg != NULL)
			{
				//fan_setting.temp_case = atoi(optarg);
				strcpy(console_manager_serial_output,optarg);

				//console_m_err("console_manager arg:O %s\n",optarg);
			}
			break;
			//printf("T case optarg is %s temp_case: %d \n",optarg,fan_setting.temp_case);
		case '?':
		case 'h':
		default:
			usage(prog);
			break;
		}
	}

	lock_file = open(console_manager_lock_file, O_CREAT | O_RDWR, 0666);
	lock_res = flock(lock_file, LOCK_EX | LOCK_NB);
	if (lock_res && EWOULDBLOCK == errno)
	{
		console_m_err("console_manager already running\n"); // another instance is running
		return -1;
	}

#ifdef DEBUG
	ubmc_log_fd = open(CONSOLE_MANAGER_SERIAL_UBMC_LOG, O_CREAT | O_RDWR |O_APPEND);
	if (ubmc_log_fd < 0)
	{
		console_m_err("console_manager error opening %s: %s\n",CONSOLE_MANAGER_SERIAL_UBMC_LOG, strerror (errno));
		return IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED;
	}
#endif

	if (console_manager_serial_source_open(&serial_source_fd1, &serial_source_fd2, &serial_out_fd, &host_log_fd)
			!= IS_CON_MGR_ERR_NULL)
	{
		console_m_err("console_manager fail to open serial_source\n");
		return IS_CON_MGR_ERR_OPEN_DEVICE_FILE_FIALED;
	}

	if ((res = console_manager_socket_init(&server_socket_fd)) != IS_CON_MGR_ERR_NULL)
	{
		console_m_err("console_manager fail to init socket \n");
		return res;
	}

	//ttyF0 seral source
#ifdef DEBUG
	if((res = console_manager_serial_source_init(serial_s_uart_sim,serial_source_fd1,serial_out_fd,-1,ubmc_log_fd) != IS_CON_MGR_ERR_NULL))
#else
	if ((res = console_manager_serial_source_init(serial_s_uart_sim, serial_source_fd1, serial_out_fd, -1, -1)
			!= IS_CON_MGR_ERR_NULL))
#endif
	{
		console_m_err("console_manager fail to init socket serial_source_1 \n");
		return res;
	}

	//host seral source
	if ((res = console_manager_serial_source_init(serial_s_tty_n_1, serial_source_fd2, serial_out_fd, server_socket_fd,
			host_log_fd) != IS_CON_MGR_ERR_NULL))
	{
		console_m_err("console_manager fail to init socket serial_source_2 \n");
		return res;
	}

	if ((res = console_manager_shm_init(&shm_p)) != IS_CON_MGR_ERR_NULL)
	{
		console_m_err("console_manager fail to init Share Memory\n");
		return res;
	}


	if (!is_cons_mgr_mgmtd_client_init(&g_is_console))
	{
		console_m_notice("load configure from mgmtd successfully");
		/*
		 *load configure from mgmtd successfully
		 */
		g_is_console.is_initalized = true;
	}
	else
	{
		/*
		 *fail to load configure from mgmtd, set the default console configuration
		 */

		console_m_err("console_manager fail to load configure from mgmtd\n");

		set_dft_console(); // set local configuration
	}

	/* apply the console configuration */
	console_manager_host_log_config_load();
	console_manager_host_serial_config_load();
	/*
	 *reopen serial before setting tty parameters
	 */
	console_manager_serial_source_host_restart();
	host_serial_fd = g_con_mgr_serial_dev[serial_s_tty_n_1].clients[serial_source_in_pollfd].fd;
	console_manager_tty_set_serial_config(host_serial_fd, &serial_config_param);

	/*
	 *if not ,Sol client socket exiting unexpectedly leads to
	 *server socket of console manager crash
	 */
	signal(SIGPIPE, console_manager_signal_handler);

	current_serial_source = serial_s_tty_n_1;
	serial_source = current_serial_source;
	shm_p[shm_pos_serial_source] = serial_source;

	while (1)
	{

		if (serial_source != current_serial_source)
		{
			console_manager_do_switch_serial_source(&serial_source, &current_serial_source);
		}

		console_manager_output_switching_serial_source_msg_tips(serial_source);

		console_manager_do_main(serial_source);

	}
	return 0;
}
