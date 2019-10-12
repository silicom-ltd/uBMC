/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "is_mgmtd_func_def.h"

#define IS_MGMTD_SNMP_THRESHOLD_PATH	"/config/snmp/threshold"

#define IS_SENSOR_TEMP_THREHOLD_MIN		60
#define IS_SENSOR_TEMP_THREHOLD_MAX		90

#define IS_FAN_SPEED_LOW_THREHOLD_MIN		100
#define IS_FAN_SPEED_LOW_THREHOLD_MAX		10000

#define IS_FAN_SPEED_HIGH_THREHOLD_MIN		10000
#define IS_FAN_SPEED_HIGH_THREHOLD_MAX		40000

int is_mgmtd_threshold_check_modify(silc_mgmtd_node* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node;
	silc_mgmtd_node* p_sub_node;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SNMP_THRESHOLD_PATH"/sensor");
	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		if (p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		{
			if (p_sub_node->tmp_value.val.uint32_val < IS_SENSOR_TEMP_THREHOLD_MIN ||
					p_sub_node->tmp_value.val.uint32_val > IS_SENSOR_TEMP_THREHOLD_MAX)
			{
				SILC_ERR("Invalid sensor temperature %s threshold %u", p_sub_node->name, p_sub_node->tmp_value.val.uint32_val);
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
			}
		}
	}

	p_sub_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SNMP_THRESHOLD_PATH"/fan/min");
	if (p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		if (p_sub_node->tmp_value.val.uint32_val < IS_FAN_SPEED_LOW_THREHOLD_MIN ||
				p_sub_node->tmp_value.val.uint32_val > IS_FAN_SPEED_LOW_THREHOLD_MAX)
		{
			SILC_ERR("Invalid fan speed min threshold %u", p_sub_node->tmp_value.val.uint32_val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	p_sub_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SNMP_THRESHOLD_PATH"/fan/max");
	if (p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		if (p_sub_node->tmp_value.val.uint32_val < IS_FAN_SPEED_HIGH_THREHOLD_MIN ||
				p_sub_node->tmp_value.val.uint32_val > IS_FAN_SPEED_HIGH_THREHOLD_MAX)
		{
			SILC_ERR("Invalid fan speed max threshold %u", p_sub_node->tmp_value.val.uint32_val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	return 0;
}

int is_mgmtd_threshold_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;

	if(type == SILC_MGMTD_IF_REQ_CHECK_MODIFY)
		return is_mgmtd_threshold_check_modify(p_node, conn_entry);
	else if(type != SILC_MGMTD_IF_REQ_MODIFY)
		return 0;

	if(strcmp(p_node->p_parent->name, "threshold") == 0 ||
			strcmp(p_node->p_parent->p_parent->name, "threshold") == 0)
	{
		silc_mgmtd_if_server_notify("/notify/snmp/threshold_changed");
	}

    return 0; 
}

