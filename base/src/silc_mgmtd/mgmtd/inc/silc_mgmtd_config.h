
#ifndef SILC_MGMTD_CONFIG_H_
#define SILC_MGMTD_CONFIG_H_

#include "silc_mgmtd_variables.h"

#ifdef __cplusplus
extern "C" {
#endif

void silc_mgmtd_cfg_init(const silc_cstr cfg_filename);
int silc_mgmtd_cfg_save_running_config_to_file(silc_cstr filename);
int silc_mgmtd_cfg_save_config_to_file();
int silc_mgmtd_cfg_save_as_config_to_file(silc_cstr filename);
int silc_mgmtd_cfg_remove_config_file(silc_cstr filename);
int silc_mgmtd_cfg_load_config_file(silc_cstr filename);
int silc_mgmtd_cfg_load_default_config_file();
int silc_mgmtd_cfg_get_config_file_list(silc_cstr buff, int* p_buff_len);
int silc_mgmtd_cfg_load_config_from_file();
void silc_mgmtd_cfg_add_dft_admin_to_running();
//int silc_mgmtd_cfg_delete_config_from_default_file(const silc_cstr delete_path);
//int silc_mgmtd_cfg_add_config_into_default_file(silc_mgmtd_node* p_node);

silc_cstr silc_mgmtd_cfg_get_config_filename();
silc_cstr silc_mgmtd_cfg_get_running_config_filename();

void silc_mgmtd_set_default_admin(silc_cstr val);
silc_cstr silc_mgmtd_get_default_admin();
void silc_mgmtd_set_default_admin_shadow(silc_cstr val);
silc_cstr silc_mgmtd_get_default_admin_shadow();

silc_cstr silc_mgmtd_get_devname();
int silc_mgmtd_get_ver_from_file(silc_cstr filename, silc_cstr version);
void silc_mgmtd_update_ts(silc_cstr module, void* conn_entry);

#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_CONFIG_H_ */
