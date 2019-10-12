/* this file is created by silc_mgmtd_inst_code_gen.py */ 
#include "silc_mgmtd_error.h"
#include "silc_mgmtd_operation.h"
#include "silc_mgmtd_config.h"
#include "silc_mgmtd_daemon.h"

#include "is_mgmtd_func_def.h"

#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include "silc_common/silc_thread.h"

#define IS_WTD_DEV				"/dev/watchdog"
#define IS_WTD_TIMEOUT			60
#define IS_WTD_KICK_INTERVAL	10

void* is_mgmtd_wtd_loop(void* thread_arg)
{
	int fd;
	int interval = IS_WTD_TIMEOUT;

	fd = open(IS_WTD_DEV, O_RDWR);
	if(-1 != fd)
	{
		if(ioctl(fd, WDIOC_SETTIMEOUT, &interval) != 0)
		{
			SILC_ERR("Fail to set watchdog timeout %s", IS_WTD_DEV);
		}
	}
	else
		SILC_ERR("Fail to open watchdog device %s", IS_WTD_DEV);

	while(1)
	{
		//SILC_LOG("Management Daemon kick watchdog");
		if(-1 != fd)
			ioctl(fd, WDIOC_KEEPALIVE, NULL);
		else
			system("touch "IS_WTD_DEV);
		sleep(IS_WTD_KICK_INTERVAL);
	}

	if(-1 != fd)
		close(fd);
	return NULL;
}

void is_mgmtd_wtd_start()
{
#ifndef SILC_MGMTD_LOCAL_DEBUG

	if(access(IS_WTD_DEV, F_OK) != 0)
	{
		SILC_INFO("Watchdog device %s not found, will not start watchdog", IS_WTD_DEV);
		return;
	}
	silc_thread_create_detached(is_mgmtd_wtd_loop, NULL);
#endif
}

void is_mgmtd_mark_started(void)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	int fd;

	fd = open("/tmp/mgmtd_started", O_CREAT|O_WRONLY, 777);
	if(fd<0)
	{
		SILC_ERR("Failed to create management daemon start flag");
		return;
	}
	close(fd);
#endif
}

int main(int argc, const char** argv)
{
	silc_time_lib_init();

	//try start watchdog if the device file exist.
	//the device file is standard in linux
	is_mgmtd_wtd_start();

	SILC_LOG("Management Daemon is starting");

	if(argc > 1 && strcmp(argv[1], "--dryrun") == 0)
	{
		SILC_LOG("Management Daemon is in dryrun mode");
		silc_mgmtd_enable_dryrun();
	}

	if(is_mgmtd_memdb_init() < 0)
	{
		SILC_ERR("Failed to create configuration database")
		goto ERROR_OUT;
	}

#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_mgmtd_cfg_init("/config/is_mgmtd.conf");
#else
	silc_mgmtd_cfg_init("is_mgmtd.conf");
#endif
	if(silc_mgmtd_cfg_load_config_from_file() < 0)
	{
		SILC_ERR("Failed to load configuration database")
		goto ERROR_OUT;
	}

	if(silc_mgmtd_daemon_start("127.0.0.1", 2626) < 0)
	{
		SILC_ERR("Failed to start mgmtd daemon")
		goto ERROR_OUT;
	}

	is_mgmtd_mark_started();
	SILC_LOG("Management Daemon is up!");

	while(1)
		sleep(1);

	return 0;
ERROR_OUT:
	SILC_ERR("Error info: %s", silc_mgmtd_err_str());
	return -1;
}

