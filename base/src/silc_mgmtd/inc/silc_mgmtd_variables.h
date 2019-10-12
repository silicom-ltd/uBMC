
#ifndef SILC_MGMTD_VARIABLES_H_
#define SILC_MGMTD_VARIABLES_H_

#include "silc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SILC_MGMTD_VAR_VAL_STR_MAX_LEN	4096

typedef enum
{
	SILC_MGMTD_VAR_NULL = 0,
	SILC_MGMTD_VAR_UINT32,
	SILC_MGMTD_VAR_INT32,
	SILC_MGMTD_VAR_UINT64,
	SILC_MGMTD_VAR_INT64,
	SILC_MGMTD_VAR_FLOAT,
	SILC_MGMTD_VAR_DOUBLE,
	SILC_MGMTD_VAR_BOOL,
	SILC_MGMTD_VAR_STRING,
	SILC_MGMTD_VAR_STRING_LINES,
	SILC_MGMTD_VAR_MACADDR,
	SILC_MGMTD_VAR_IPV4ADDR,
	SILC_MGMTD_VAR_IPV6ADDR,
	SILC_MGMTD_VAR_IPADDR,
	SILC_MGMTD_VAR_DATETIME,
	SILC_MGMTD_VAR_DATE,
	SILC_MGMTD_VAR_TIME,
	SILC_MGMTD_VAR_HEX,
	SILC_MGMTD_VAR_MAX,
}silc_mgmtd_var_type;

typedef struct silc_mgmtd_var_hex_s
{
	uint8_t*	data;
	int			len;
}silc_mgmtd_var_hex;

typedef struct silc_ipaddr_s
{
	int af;
	union {
		uint32_t		v4;
		struct in6_addr v6;
	} addr;
} silc_ipaddr;

typedef struct silc_mgmtd_var_s
{
	silc_mgmtd_var_type type;
	union{
		uint32_t	uint32_val;
		int32_t		int32_val;
		uint64_t	uint64_val;
		int64_t		int64_val;
		float		float_val;
		double		double_val;
		silc_bool		bool_val;
		silc_cstr		string_val;
		silc_ipaddr ipaddr_val;
		uint32_t	date_val;
		uint32_t	time_val;
		uint32_t	datetime_val;
		uint8_t		macaddr_val[6];
		silc_mgmtd_var_hex	hex_val;
	}val;
}silc_mgmtd_var;


/**
 * Get variables type string name.
 */
const silc_cstr silc_mgmtd_var_get_type_name(silc_mgmtd_var_type type);

/**
 * Get variables type by string name.
 */
silc_mgmtd_var_type silc_mgmtd_var_get_type(const silc_cstr name);

/**
 * Convert variables value string.
 */
silc_cstr silc_mgmtd_var_to_str(silc_mgmtd_var* p_var, silc_cstr buff, int buff_len);

silc_cstr silc_mgmtd_var_uint32_to_str(uint32_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_int32_to_str(int32_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_uint64_to_str(uint64_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_int64_to_str(int64_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_float_to_str(float value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_double_to_str(double value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_bool_to_str(silc_bool value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_boolean_to_str(silc_bool value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_str_to_str(const silc_cstr value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_mac_addr_to_str(uint8_t* value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_ipv4_addr_to_str(uint32_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_ipv6_addr_to_str(struct in6_addr* value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_ip_addr_to_str(silc_ipaddr* value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_datetime_to_str(uint32_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_date_to_str(uint32_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_time_to_str(uint32_t value, silc_cstr str_buf, int buf_len);
silc_cstr silc_mgmtd_var_hex_to_str(silc_mgmtd_var_hex* value, silc_cstr str_buf, int buf_len);
int silc_mgmtd_var_get_val_str_min_len(silc_mgmtd_var* p_var);

/*
 * Set variables value by string
 */
int silc_mgmtd_var_set_by_str(silc_mgmtd_var* p_var, const silc_cstr val_str);

/*
 * initialize variables
 */
int silc_mgmtd_var_init(silc_mgmtd_var* p_var, silc_mgmtd_var_type type, const silc_cstr val_str);

/*
 * clear variables
 */
void silc_mgmtd_var_clear(silc_mgmtd_var* p_var);

/*
 * copy variables
 */
int silc_mgmtd_var_copy(silc_mgmtd_var* p_src_var, silc_mgmtd_var* p_dst_var);

/*
 * compare variables
 */
silc_bool silc_mgmtd_var_equal(silc_mgmtd_var* p_var0, silc_mgmtd_var* p_var1);


#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_VARIABLES_H_ */
