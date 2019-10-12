#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#include "ubmc_common.h"
#include "ubmc_state.h"
#include "ubmc_shm_state.h"


ubmc_shm_state* ubmc_shm_state_create(void)
{
	int ret = 0;
	struct stat st;
	ubmc_shm_state* p_ret = NULL;
	char* p_tmp = NULL;

	is_call_create(p_ret, malloc, sizeof(ubmc_shm_state));
	memset(p_ret, 0, sizeof(*p_ret));
	p_ret->fd = -1;

	if(stat(UBMC_SHM_FILE, &st)==0)
		is_call_init(open, UBMC_SHM_FILE, O_RDWR);
	else
		is_call_init(open, UBMC_SHM_FILE, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
	p_ret->fd = ret;

	is_call_create(p_tmp, malloc, sizeof(ubmc_ipmi_state));
	memset(p_tmp, 0, sizeof(ubmc_ipmi_state));
	ret = write(p_ret->fd, p_tmp, sizeof(ubmc_ipmi_state));
	if(ret != sizeof(ubmc_ipmi_state))
	{
		is_err("Failed to setup shm for switch state, %d", ret);
		goto ERR_RET;
	}

	is_call_create(p_ret->map_addr, mmap, NULL, sizeof(ubmc_ipmi_state), PROT_READ|PROT_WRITE, MAP_SHARED, p_ret->fd, 0);

	memset(p_ret->map_addr, 0, sizeof(ubmc_ipmi_state));

	free(p_tmp);
	return p_ret;

ERR_RET:
	if(p_ret)
	{
		if(p_ret->fd >=0 )
			close(p_ret->fd);
		free(p_ret);
	}
	if(p_tmp)
		free(p_tmp);
	return NULL;
}
