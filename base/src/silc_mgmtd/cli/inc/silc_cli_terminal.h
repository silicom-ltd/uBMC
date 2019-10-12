
#ifndef SILC_CLI_TERMINAL_H_
#define SILC_CLI_TERMINAL_H_

#include "silc_common.h"

#ifdef __cplusplus
extern "C" {
#endif


extern void silc_cli_terminal_init(int max_line, int max_col);
extern void silc_cli_terminal_get_max();
extern void silc_cli_terminal_set_max(int max_line, int max_col);
extern void silc_cli_terminal_clear();


#define SILC_CLI_TERMINAL_PRINT_MAX	1024*16
extern char g_silc_cli_terminal_print_buff[SILC_CLI_TERMINAL_PRINT_MAX];
void silc_cli_terminal_print_inner(char* str, int n);

#define silc_cli_terminal_print(fmt, ...)	\
	do{ \
		int n = sprintf(g_silc_cli_terminal_print_buff, fmt, ## __VA_ARGS__); \
		if(n <= SILC_CLI_TERMINAL_PRINT_MAX) \
			silc_cli_terminal_print_inner(g_silc_cli_terminal_print_buff, n); \
	}while(0)

#define silc_cli_print 	printf //silc_cli_terminal_print


#ifdef __cplusplus
}
#endif

#endif /* SILC_CLI_TERMINAL_H_ */
