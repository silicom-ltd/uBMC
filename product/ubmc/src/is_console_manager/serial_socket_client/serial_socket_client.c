#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/un.h>
#include <string.h>
#include "../console_manager/console_manager_cmdline.h"

#define ADDRESS     "/var/run/console_manager_socket"  /* addr to connect */

#define get_bit_status(x,y)  (x & 1 << y)

#define set_bit_status(x,y)  (x |= 1 << y)

#define K_ERA       '\b'
#define K_CTRL_EXIT		0x1d
#define NUM_KEYS    23
#define KEY_OFFS    256
#define BUFFERSIZE 2048

static int keys_in_buf;
static char erasechar;
int pendingkeys = 0;
int io_pending = 0;
/* Internal structure. */
struct key
{
	char *cap;
	char len;
};

enum
{
	CONSOLE_MANAGER_STDIN_FD = 0, CONSOLE_MANAGER_STDOUT_FD,
};

enum
{
	IS_CONSOLE_MANAGER_ERR_NULL = 0,
	IS_CONSOLE_MANAGER_ERR_ALLOC_MEM_FIALED,
	IS_CONSOLE_MANAGER_ERR_SERVER_SOCKET,
	IS_CONSOLE_MANAGER_ERR_SERVER_BIND,
	IS_CONSOLE_MANAGER_ERR_SERVER_LISTEN,
	IS_CONSOLE_MANAGER_ERR_OPEN_DEVICE_FILE_FIALED,
	IS_CONSOLE_MANAGER_ERR,
};

enum
{
	CONSOLE_MANAGER_TERMINAL_STATUS, CONSOLE_MANAGER_SERIAL_DEV_STATUS,CONSOLE_MANAGER_SERIAL_STATUS_MAX,
};

struct terminal_io
{
	char _bufstart[BUFFERSIZE];
	char *_bufpos;
	char *_buffend;
};

static struct terminal_io terminal_buf;

void console_m_serial_socket_client_terminal_buf_init()
{

	memset(terminal_buf._bufstart, 0, BUFFERSIZE);
	terminal_buf._bufpos = terminal_buf._bufstart;
	terminal_buf._buffend = terminal_buf._bufstart + BUFFERSIZE;

}

int console_m_serial_socket_client_set_char_break(int mode)
{
	struct termios tty;
	static int init = 0;
	static int erasechar;
	struct termios savetty;

	if (init == 0)
	{
		tcgetattr(CONSOLE_MANAGER_STDIN_FD, &savetty);
		erasechar = savetty.c_cc[VERASE];
		init++;
	}

	if (mode == 3)
		return erasechar;

	/* Always return to default settings first */
	tcsetattr(CONSOLE_MANAGER_STDIN_FD, TCSADRAIN, &savetty);

	if (mode == 0)
	{
		return erasechar;
	}

	tcgetattr(CONSOLE_MANAGER_STDIN_FD, &tty);
	if (mode == 1)
	{
		tty.c_oflag &= ~OPOST;
		tty.c_lflag &= ~(XCASE | ECHONL | NOFLSH);
		tty.c_lflag &= ~(ICANON | ISIG | ECHO);
		tty.c_iflag &= ~(ICRNL | INLCR);
		tty.c_cflag |= CREAD;
		tty.c_cc[VTIME] = 5;
		tty.c_cc[VMIN] = 1;
	}
	if (mode == 2)
	{ /* raw */
		tty.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
		IXANY | IXON | IXOFF | INPCK | ISTRIP);
		tty.c_iflag |= (BRKINT | IGNPAR);
		tty.c_lflag &= ~(XCASE | ECHONL | NOFLSH);
		tty.c_lflag &= ~(ICANON | ISIG | ECHO);
		tty.c_cflag |= CREAD;
		tty.c_cc[VTIME] = 5;
		tty.c_cc[VMIN] = 1;
	}
	tcsetattr(CONSOLE_MANAGER_STDIN_FD, TCSADRAIN, &tty);
	return erasechar;
}

int flag = 0;
/* Check if there is IO pending. */
int console_m_serial_socket_client_check_io(int fd1, int fd2,int tmout, char *buf, int bufsize, int *bytes_read)
{
	int n = 0, i;
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = tmout / 1000;
	tv.tv_usec = (tmout % 1000) * 1000L;

	i = fd1;
	if (fd2 > fd1)
		i = fd2;

	FD_ZERO(&fds);
	if (fd1 >= 0)
		FD_SET(fd1, &fds);
	else
		fd1 = 0;

	if (fd2 >= 0)
		FD_SET(fd2, &fds);
	else
		fd2 = 0;

	if (fd2 == 0 && io_pending)
	{
		set_bit_status(n, CONSOLE_MANAGER_SERIAL_DEV_STATUS);

	}
	else if (select(i + 1, &fds, NULL, NULL, &tv) > 0)
	{
		if (FD_ISSET(fd1, &fds) > 0)
		{
			set_bit_status(n, CONSOLE_MANAGER_TERMINAL_STATUS);
		}
		if (FD_ISSET(fd2, &fds) > 0)
		{
			set_bit_status(n, CONSOLE_MANAGER_SERIAL_DEV_STATUS);
		}

	}
	/* If there is data put it in the buffer. */
	if (buf)
	{
		i = 0;
		if ((n & 1) == 1)
		{
			i = read(fd1, buf, bufsize - 1);
		}
		buf[i > 0 ? i : 0] = 0;
		if (bytes_read)
			*bytes_read = i;
	}

	return n;
}

/*
 * Function to read chunks of data from fd 0 all at once
 */

static int console_m_serial_socket_client_read_char_from_terminal(char *c)
{
	static char buf[32];
	static int idx = 0;
	static int lastread = 0;

	if (idx > 0 && idx < lastread)
	{
		*c = buf[idx++];
		keys_in_buf--;
		if (keys_in_buf == 0 && pendingkeys == 0)
			io_pending = 0;
		return lastread;
	}
	idx = 0;
	do
	{
		lastread = read(CONSOLE_MANAGER_STDIN_FD, buf, 32);
		keys_in_buf = lastread - 1;
	} while (lastread < 0 && errno == EINTR);

	*c = buf[0];
	if (lastread > 1)
	{
		idx = 1;
		io_pending++;
	}

	return lastread;
}

int console_m_serial_socket_client_get_char_from_terminal(void)
{
	int f, g;
	int match = 1;
	int len;
	char c;
	static unsigned char mem[8];
	static int leftmem = 0;
	static int init = 0;
	int nfound = 0;
	struct timeval timeout;
	fd_set readfds;

	/* Some sequence still in memory ? */
	if (leftmem > 0)
	{
		leftmem--;
		if (leftmem == 0)
			pendingkeys = 0;
		if (pendingkeys == 0 && keys_in_buf == 0)
			io_pending = 0;
		return mem[leftmem];
	}
	pendingkeys = 0;

	for (len = 1; len < 8 && match; len++)
	{
		if (len > 1 && keys_in_buf == 0)
		{
			timeout.tv_sec = 0;
			timeout.tv_usec = 400000; /* 400 ms */
			FD_ZERO(&readfds);
			FD_SET(0, &readfds);
			if (!(nfound = select(CONSOLE_MANAGER_STDOUT_FD, &readfds, NULL, NULL, &timeout)))
				break;
		}
		while ((nfound = console_m_serial_socket_client_read_char_from_terminal(&c)) < 0 && (errno == EINTR))
			;

		if (nfound < 1)
			return EOF;

		if (len == 1)
		{
			/* Enter and erase have precedence over anything else */
			if (c == '\n')
				return c;
			if (c == erasechar)
			{
				return K_ERA;
			}
			if(c == K_CTRL_EXIT)
			{
				return K_CTRL_EXIT;
			}
		}

		mem[len - 1] = c;
		match = 0;
	}

	len--; /* for convenience */
	if (len == 1)
		return mem[0];
	/* Remember there are more keys waiting in the buffer */
	pendingkeys++;
	io_pending++;

	/* Reverse the "mem" array */
	for (f = 0; f < len / 2; f++)
	{
		g = mem[f];
		mem[f] = mem[len - f - 1];
		mem[len - f - 1] = g;
	}

	leftmem = len - 1;
	return mem[leftmem];

}

void console_m_put_terminal_buf_to(int fd)
{
	int todo, done;
	char *buf;

	todo = terminal_buf._bufpos - terminal_buf._bufstart;

	terminal_buf._bufpos = terminal_buf._bufstart;
	buf = terminal_buf._bufstart;

	while (todo > 0)
	{
		done = write(fd, buf, todo);
		if (done > 0)
		{
			todo -= done;
			buf += done;
		}
		if (done < 0 && errno != EINTR)
			break;
	}

}

static int console_m_serial_socket_client_output_char(int c)
{

	*terminal_buf._bufpos = c;
	terminal_buf._bufpos++;

	if (terminal_buf._bufpos >= terminal_buf._buffend)
		console_m_put_terminal_buf_to(CONSOLE_MANAGER_STDOUT_FD);
	return 0;
}


int console_m_serial_socket_client_flush_to_serial_dev(int client_socket_fd)
{
	int c = 0;

	/* See which key was pressed. */
	c = console_m_serial_socket_client_get_char_from_terminal();
	if (c == EOF)
		return EOF;
	if (c == K_CTRL_EXIT)
		return K_CTRL_EXIT;

	write(client_socket_fd, &c, 1);
	return 0;
}

int console_m_serial_socket_client_socket_init(int * client_socket_fd)
{

	int len;
	int flags;
	struct sockaddr_un saun;

	if ((*client_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("client: socket");
		return -1;
	}

	/*set non-blocking mode no socket*/
	flags = fcntl(*client_socket_fd, F_GETFL, 0);
	fcntl(*client_socket_fd, F_SETFL, flags | O_NONBLOCK);

	/* Create the address we will be connecting to.*/
	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path, ADDRESS);

	/*
	 * Try to connect to the address.  For this to
	 * succeed, the server must already have bound
	 * this address, and must have issued a listen()
	 * request.
	 */
	len = sizeof(saun.sun_family) + strlen(saun.sun_path);

	if (connect(*client_socket_fd, (struct sockaddr *) &saun, len) < 0)
	{
		perror("client: connect");
		return -1;
	}

	return 0;

}


int console_m_serial_socket_client_flush_to_terminal(char *buf, int len)
{
	char *ptr;
	ptr = buf;

	while (len-- > 0)
	{
		console_m_serial_socket_client_output_char(*ptr++);
	}
	console_m_put_terminal_buf_to(CONSOLE_MANAGER_STDOUT_FD);
	return 0;
}

void console_m_send_socket_kick_out_msg(int msgid)
{
	struct mesg_buffer message;

	message.mesg_type = CLI_CMD_TYPE_KICK_OUT_SOCKET;

	strcpy(message.mesg_text, "kick out socket client");

	// msgsnd to send message
	msgsnd(msgid, &message, sizeof(message), 0);

}

int console_m_shm_init(int ** shm)
{
	int shmid;

	/*
	 * Locate the segment.
	 */
	if ((shmid = shmget(shm_key, SHMSZ, 0666)) < 0)
	{
		perror("shmget");
		return shmid;
	}

	/*
	 * Now we attach the segment to our data space.
	 */
	if ((*shm = shmat(shmid, NULL, 0)) == (int *) -1)
	{
		perror("shmat");
		return -1;
	}

	return 0;

}

#define HOST_SERIAL_DEV	"/dev/ttyS1"
#define BMC_SERIAL_DEV	"/dev/ttyS0"
#define BMC_SERIAL	"/dev/ttyF0"

int console_m_get_tty(char *cur_tty)
{
	int ret = -1 ;
	int rc;
    FILE *fp = NULL;
    char buffer[64] = { 0 };
    char cmd[128] = { 0 };
    char *tmp = NULL;
	sprintf(cmd,"tty 2>/dev/null ");
	fp=popen(cmd,"r");
	if( fp == NULL )
	{
		printf("popen Error! :%s\n",strerror(errno));
		return ret;
	}
	memset(cur_tty,0,sizeof(cur_tty));
    tmp = fgets(buffer,sizeof(buffer),fp);
    if(buffer[strlen(buffer)-1] == '\n')
    	buffer[strlen(buffer)-1] = '\0';
    strcpy(cur_tty,buffer);
	// the result of the shell command is null
exit:
    rc = pclose(fp);
    //in below check the cmd is done successfully
    if(rc == -1)
    {
    	//do command fail
    	return -1;
    }
    if (WIFEXITED(rc))
    {
    	//printf("subprocess exited, exit code: %d\n", WEXITSTATUS(rc));
    	if (0 == WEXITSTATUS(rc))
    	{
    		//printf("command succeed\n");
    		ret = 0;
    	}
    	else
    	{
    		if(127 == WEXITSTATUS(rc))
    		{
    			//can not find this cmd
    			ret = -1;
    		}
    		else
    		{
    			//printf("command:%s failed: %s\n",cmd,strerror(WEXITSTATUS(rc)));
    			ret = -1;
    		}
    	}
    }
    else
    {
    	//printf("subprocess exit failed\n");
    	ret = -1;
    }
	return ret ;
}

void usage(char * prog)
{
	fprintf(stderr, "usage: %s [-h] [-f]\n -h : help \n -f : force to kick out existing socket connection \n", prog);
	exit(-1);
}

int main(int argc, char *argv[])
{

	char buf[128];
	char cur_tty[64];
	int buf_offset = 0;
	int blen;
	int x = 0;
	int msgid;
	int rv;
	int i, client_socket_fd;

	char * prog;
	int opt;
	int * shm_p;
	int count = 0;

	prog = argv[0];

	rv = console_m_get_tty(cur_tty);
	if(rv < 0 || strncmp(cur_tty,BMC_SERIAL,strlen(cur_tty)) == 0)
	{
		fprintf(stderr,"Not support this command in serial,please input ctrl + x switch to host ");
		return -1;
	}
	//signal(SIGPIPE,SIG_IGN);
	if (console_m_shm_init(&shm_p) < 0)
	{
		perror("server is not ready,please start console manager firstly");
		return -1;
	}

	// msgget creates a message queue
	// and returns identifier
	msgid = msgget(serial_source_2_msg_queue_key, 0666 | IPC_CREAT);
	if (msgid < 0)
	{
		perror("server is not ready,please start console manager firstly");
		return msgid;
	}

	while ((opt = getopt(argc, argv, "hf")) > 0)
	{
		switch (opt)
		{
		case 'f':
			if (shm_p[shm_pos_serial_source] == serial_s_uart_sim)
			{
				shm_p[shm_pos_serial_source] = serial_s_tty_n_1;
				shm_p[shm_pos_status] = shm_op_st_processing;
				do
				{
					/*
					 *waiting console manager switch serial source
					 */
					usleep(200000);

					/*
					 *over 100s ,timeout and exit
					 */
					if (++count > 5000)
					{
						shm_p[shm_pos_status] = shm_op_st_null;
						shm_p[shm_pos_serial_source] = serial_s_uart_sim;
						perror("the console manager is no response , exiting ...");
						exit(-1);
					}

				} while (shm_p[shm_pos_status] != shm_op_st_confirm);
				//clear status
				shm_p[shm_pos_status] = shm_op_st_null;
			}

			console_m_send_socket_kick_out_msg(msgid);
			break;
		case 'h':
		default:
			usage(prog);
			break;
		}
	}

/*	if (*shm_p == serial_s_uart_sim)
	{
		printf("please login with paramter force and retry\n");
		return 0;
	}
*/
	if (console_m_serial_socket_client_socket_init(&client_socket_fd) < 0)
	{
		perror("server is not ready,please start console manager firstly");
		return -1;
	}

	console_m_serial_socket_client_terminal_buf_init();

	/*it is used to set terminal stdin no caching*/
	console_m_serial_socket_client_set_char_break(2);
	int len;
	int host_ser_fd;
	int fd_instance;

	char *read_buf[128] = {0};
	int ret = 0;
	int c ;
	host_ser_fd = open(HOST_SERIAL_DEV, O_RDWR | O_NOCTTY | O_SYNC,S_IRWXU|S_IRWXO);
	if(host_ser_fd < 0)
	{
		perror("open HOST_SERIAL_DEV fail ");
	}
	fprintf(stderr,"Press  Ctrl+']' if you want to exit.\n");
	while (1)
	{

		fd_instance = client_socket_fd ;
		x = console_m_serial_socket_client_check_io(fd_instance, 0,1000, buf, sizeof(buf), &blen);
		if (get_bit_status(x, CONSOLE_MANAGER_TERMINAL_STATUS))
		{
			console_m_serial_socket_client_flush_to_terminal(buf, blen);
		}
		if (get_bit_status(x, CONSOLE_MANAGER_SERIAL_DEV_STATUS))
		{
			ret = console_m_serial_socket_client_flush_to_serial_dev(fd_instance);
			if(ret == K_CTRL_EXIT)
			{
				break;
			}
		}

	}

	close(client_socket_fd);
	return 0;
}
