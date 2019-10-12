#include <stdio.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/un.h>

#define HOST_SERIAL_SOURCE "/dev/ttyS1"

#define ADDRESS     "console_manager_socket"  /* addr to connect */

#define get_bit_status(x,y)  (x & 1 << y)

#define set_bit_status(x,y)  (x |= 1 << y)

#define K_ERA       '\b'
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
	IS_CONSOLE_MANAGER_ERR_EXCEPTION = -1,
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
	CONSOLE_MANAGER_TERMINAL_STATUS, CONSOLE_MANAGER_SERIAL_DEV_STATUS, CONSOLE_MANAGER_SERIAL_STATUS_MAX,
};

struct terminal_io
{
	char _bufstart[BUFFERSIZE];
	char *_bufpos;
	char *_buffend;
};

static struct terminal_io terminal_buf;

void console_m_serial_host_terminal_buf_init()
{

	memset(terminal_buf._bufstart, 0, BUFFERSIZE);
	terminal_buf._bufpos = terminal_buf._bufstart;
	terminal_buf._buffend = terminal_buf._bufstart + BUFFERSIZE;

}

int console_m_serial_host_set_char_break(int mode)
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
		tty.c_oflag &= ~OPOST;
		tty.c_lflag &= ~(XCASE | ECHONL | NOFLSH);
		tty.c_lflag &= ~(ICANON | ISIG | ECHO);
		tty.c_cflag |= CREAD;
		tty.c_cc[VTIME] = 5;
		tty.c_cc[VMIN] = 1;
	}
	tcsetattr(CONSOLE_MANAGER_STDIN_FD, TCSADRAIN, &tty);
	return erasechar;
}

void console_m_serial_host_tty_set_blocking(int fd, int should_block)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0)
	{
		printf("error from tggetattr \n");
		return;
	}

	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
		printf("error setting term attributes \n");
}

int console_m_serial_host_tty_set_interface_attribs(int fd, int speed, int parity)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0)
	{
		printf("error from tcgetattr \n");
		return -1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
									// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN] = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_iflag &= ~(INLCR | ICRNL); //disable cr convert to newline or newline convert to cr

	tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,
									 // enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		printf("error from tcsetattr \n");
		return -1;
	}
	return 0;
}

/* Check if there is IO pending. */
int console_m_serial_host_check_io(int fd1, int fd2, int tmout, char *buf, int bufsize, int *bytes_read)
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
			set_bit_status(n, CONSOLE_MANAGER_TERMINAL_STATUS);

		if (FD_ISSET(fd2, &fds) > 0)
			set_bit_status(n, CONSOLE_MANAGER_SERIAL_DEV_STATUS);

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

static int console_m_serial_host_read_char_from_terminal(char *c)
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

int console_m_serial_host_get_char_from_terminal(void)
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
		while ((nfound = console_m_serial_host_read_char_from_terminal(&c)) < 0 && (errno == EINTR))
			;

		if (nfound < 1)
			return EOF;

		if (len == 1)
		{

			/* Enter and erase have precedence over anything else */
			if (c == '\n')
				return c;
			if (c == erasechar)
				return K_ERA;
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

void console_m_serial_host_write_to_terminal(void)
{
	int todo, done;
	char *buf;

	todo = terminal_buf._bufpos - terminal_buf._bufstart;

	terminal_buf._bufpos = terminal_buf._bufstart;
	buf = terminal_buf._bufstart;

	while (todo > 0)
	{
		done = write(CONSOLE_MANAGER_STDOUT_FD, buf, todo);
		if (done > 0)
		{
			todo -= done;
			buf += done;
		}
		if (done < 0 && errno != EINTR)
			break;
	}

}

static int console_m_serial_host_output_char(int c)
{

	*terminal_buf._bufpos = c;
	terminal_buf._bufpos++;

	if (terminal_buf._bufpos >= terminal_buf._buffend)
		console_m_serial_host_write_to_terminal();

	return IS_CONSOLE_MANAGER_ERR_NULL;
}

int console_m_serial_host_flush_to_serial_dev(int client_socket_fd)
{
	int c = 0;

	/* See which key was pressed. */
	c = console_m_serial_host_get_char_from_terminal();
	if (c == EOF)
		return EOF;

	write(client_socket_fd, &c, 1);
	return IS_CONSOLE_MANAGER_ERR_NULL;
}

int console_m_serial_host_tty_init(int argc, char *argv[], int * fd)
{

	*fd = open(HOST_SERIAL_SOURCE, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
		printf("error opening %s: %s\n", HOST_SERIAL_SOURCE, strerror(errno));
		return IS_CONSOLE_MANAGER_ERR_EXCEPTION;
	}

	return IS_CONSOLE_MANAGER_ERR_NULL;

}

int console_m_serial_host_flush_to_terminal(char *buf, int len)
{
	char *ptr;
	ptr = buf;

	while (len-- > 0)
	{
		console_m_serial_host_output_char(*ptr++);
	}
	console_m_serial_host_write_to_terminal();
	return IS_CONSOLE_MANAGER_ERR_NULL;
}

int main(int argc, char *argv[])
{

	char buf[128];
	int blen;
	int x = 0;

	int i, fd;

	if (console_m_serial_host_tty_init(argc, argv, &fd) < 0)
		return IS_CONSOLE_MANAGER_ERR_EXCEPTION;

	console_m_serial_host_terminal_buf_init();

	console_m_serial_host_tty_set_interface_attribs(fd, B115200, 0);
	console_m_serial_host_tty_set_blocking(fd, 0);

	/*it is used to set terminal stdin no caching*/
	console_m_serial_host_set_char_break(2);

	while (1)
	{

		x = console_m_serial_host_check_io(fd, 0, 1000, buf, sizeof(buf), &blen);

		if (get_bit_status(x, CONSOLE_MANAGER_TERMINAL_STATUS))
		{

			console_m_serial_host_flush_to_terminal(buf, blen);

		}

		if (get_bit_status(x, CONSOLE_MANAGER_SERIAL_DEV_STATUS))
		{

			console_m_serial_host_flush_to_serial_dev(fd);

		}

	}

	close(fd);

	return 0;
}
