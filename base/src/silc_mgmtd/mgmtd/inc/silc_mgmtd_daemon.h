
#ifndef SILC_MGMTD_DAEMON_H_
#define SILC_MGMTD_DAEMON_H_

#include "silc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

int silc_mgmtd_daemon_start(const char* bind_addr, int listen_port);

#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_DAEMON_H_ */
