#ifndef INCLUDE_UBMC_SHM_STATE_H_
#define INCLUDE_UBMC_SHM_STATE_H_

#define UBMC_SHM_FILE		"/dev/shm/ubmc_ipmi_state"

typedef struct ubmc_shm_state_s
{
	int fd;
	void* map_addr;
}ubmc_shm_state;

ubmc_shm_state* ubmc_shm_state_create(void);

#endif /* INCLUDE_BMC_SHM_STATE_H_ */
