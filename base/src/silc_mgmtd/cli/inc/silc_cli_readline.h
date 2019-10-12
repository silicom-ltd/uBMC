#ifndef SILC_CLI_READLINE_H_
#define SILC_CLI_READLINE_H_

#include "silc_common.h"
#include "silc_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void silc_cli_rl_init(const char* name);
extern silc_cstr silc_cli_rl_readline();
extern void silc_cli_rl_add_history(silc_cstr line);
extern void silc_cli_rl_clear_history();
extern void silc_cli_rl_free_readline(silc_cstr line);

extern silc_cli_cmd* silc_cli_rl_parser_cmd(silc_cstr cmd_str, silc_list* p_token_list);
void silc_cli_rl_free_token_list(silc_list* p_token_list);

#ifdef __cplusplus
}
#endif

#endif /* SILC_CLI_READLINE_H_ */
