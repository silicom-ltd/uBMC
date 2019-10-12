#ifndef _SILC_CLI_CMD_SHOW_RUNNING_CONFIG_H_
#define _SILC_CLI_CMD_SHOW_RUNNING_CONFIG_H_

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

static inline void is_cli_show_running_config(const silc_cstr changed_config)
{
	silc_cli_print("%s", changed_config);

}

#endif
