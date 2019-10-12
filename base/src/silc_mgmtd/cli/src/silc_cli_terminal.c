
#include <termios.h>

#include "silc_cli_terminal.h"
#include "editline/readline.h"

char g_silc_cli_terminal_print_buff[SILC_CLI_TERMINAL_PRINT_MAX];

static int s_silc_cli_term_max_line;
static int s_silc_cli_term_max_col;

static int s_silc_cli_term_cur_line;
static int s_silc_cli_term_cur_col;

void silc_cli_terminal_get_max()
{
	rl_get_screen_size(&s_silc_cli_term_max_line, &s_silc_cli_term_max_col);
}

void silc_cli_terminal_set_max(int max_line, int max_col)
{
	s_silc_cli_term_max_line = max_line;
	s_silc_cli_term_max_col = max_col;
}

void silc_cli_terminal_clear()
{
	s_silc_cli_term_cur_line = 0;
	s_silc_cli_term_cur_col = 0;
}

void silc_cli_terminal_init(int max_line, int max_col)
{
	if(max_line > 0 && max_col > 0)
		silc_cli_terminal_set_max(max_line, max_col);
	else
		silc_cli_terminal_get_max();
	silc_cli_terminal_clear();
}

int silc_cli_terminal_getch(void)
{
	struct termios oldt, newt;
	int ch;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;
}

void silc_cli_terminal_print_inner(char* str, int n)
{
	char c;
	char data[4096];
	int prt_nch, cur_pos = 0;
	int len = n;

	//silc_cli_terminal_get_max();

	while(len > 0)
	{
		if((s_silc_cli_term_cur_col+len) > s_silc_cli_term_max_col)
		{
			prt_nch = s_silc_cli_term_max_col - s_silc_cli_term_cur_col;
			strncpy(data, str+cur_pos, prt_nch+1);
			data[prt_nch] = 0;
			printf("%s\n", data);
			cur_pos += prt_nch;
			len -= prt_nch;
			s_silc_cli_term_cur_col = 0;
			s_silc_cli_term_cur_line++;
		}
		else
		{
			prt_nch = len;
			strncpy(data, str+cur_pos, prt_nch+1);
			data[prt_nch] = 0;
			printf("%s", data);
			len -= prt_nch;
			s_silc_cli_term_cur_col += prt_nch;
			if(data[prt_nch-1] == '\n')
			{
				s_silc_cli_term_cur_col = 0;
				s_silc_cli_term_cur_line++;
			}
		}

		if(s_silc_cli_term_cur_line >= (s_silc_cli_term_max_line-1))
		{
			printf(" --More--");
			c = silc_cli_terminal_getch();
			printf("\033[1A\n");
			if(c != 10)
			{
				printf("           \n");
				break;
			}
		}
		fflush(stdout);
	}
}


