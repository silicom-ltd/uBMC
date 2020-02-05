

#include "silc_common.h"
#include "silc_mgmtd_lib_err.h"
#include "silc_mgmtd_variables.h"
#include "silc_mgmtd_interface.h"

#define SILC_MGMTD_VAR_NAME_MAX_LEN		32

static char s_silc_mgmtd_var_names[SILC_MGMTD_VAR_MAX][SILC_MGMTD_VAR_NAME_MAX_LEN] = {
		"NULL",
		"UINT32",
		"INT32",
		"UINT64",
		"INT64",
		"FLOAT",
		"DOUBLE",
		"BOOL"
		"STRING",
		"STRING_LINES",
		"MACADDR",
		"IPV4ADDR",
		"DATETIME",
		"DATE",
		"TIME",
		"HEX",
		"UNKNOWN",
};


const silc_cstr silc_mgmtd_var_get_type_name(silc_mgmtd_var_type type)
{
	return s_silc_mgmtd_var_names[type];
}


silc_mgmtd_var_type silc_mgmtd_var_get_type(const silc_cstr name)
{
	int loop;
	for(loop=0; loop<SILC_MGMTD_VAR_MAX; loop++)
	{
		if(strncmp(name, s_silc_mgmtd_var_names[loop], SILC_MGMTD_VAR_NAME_MAX_LEN) == 0)
			return (silc_mgmtd_var_type)loop;
	}
	silc_mgmtd_lib_err_set(LIB_ERR_VAR_TYPE_NAME_INVALID);
	return SILC_MGMTD_VAR_MAX;
}


#define silc_mgmd_var_convert_chk_len(buf_len, min_len)	\
	if(buf_len < min_len) \
	{	\
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_VAL_BUFF_NOT_ENOUGH);	\
		return NULL;	\
	}

#define SILC_MGMTD_VAR_STR_LEN_MIN_NULL		5
#define SILC_MGMTD_VAR_STR_LEN_MIN_UNKNOWN	8
#define SILC_MGMTD_VAR_STR_LEN_MIN_32BIT	12
#define SILC_MGMTD_VAR_STR_LEN_MIN_64BIT	22
#define SILC_MGMTD_VAR_STR_LEN_MIN_BOOL		6
#define SILC_MGMTD_VAR_STR_LEN_MIN_MACADDR	18
#define SILC_MGMTD_VAR_STR_LEN_MIN_IPV4ADDR	16
#define SILC_MGMTD_VAR_STR_LEN_MIN_IPV6ADDR	40
#define SILC_MGMTD_VAR_STR_LEN_MIN_DATE		11
#define SILC_MGMTD_VAR_STR_LEN_MIN_TIME		9
#define SILC_MGMTD_VAR_STR_LEN_MIN_DATETIME	19

int silc_mgmtd_var_get_val_str_min_len(silc_mgmtd_var* p_var)
{
	switch(p_var->type)
	{
	case SILC_MGMTD_VAR_NULL:
		return SILC_MGMTD_VAR_STR_LEN_MIN_NULL;
	case SILC_MGMTD_VAR_UINT32:
	case SILC_MGMTD_VAR_INT32:
	case SILC_MGMTD_VAR_FLOAT:
		return SILC_MGMTD_VAR_STR_LEN_MIN_32BIT;
	case SILC_MGMTD_VAR_UINT64:
	case SILC_MGMTD_VAR_INT64:
	case SILC_MGMTD_VAR_DOUBLE:
		return SILC_MGMTD_VAR_STR_LEN_MIN_64BIT;
	case SILC_MGMTD_VAR_BOOL:
		return SILC_MGMTD_VAR_STR_LEN_MIN_BOOL;
	case SILC_MGMTD_VAR_STRING:
		return strlen(p_var->val.string_val)+1;
	case SILC_MGMTD_VAR_STRING_LINES:
		return silc_mgmtd_if_str_escape_len(p_var->val.string_val);
	case SILC_MGMTD_VAR_MACADDR:
		return SILC_MGMTD_VAR_STR_LEN_MIN_MACADDR;
	case SILC_MGMTD_VAR_IPV4ADDR:
		return SILC_MGMTD_VAR_STR_LEN_MIN_IPV4ADDR;
	case SILC_MGMTD_VAR_IPV6ADDR:
	case SILC_MGMTD_VAR_IPADDR:
		return SILC_MGMTD_VAR_STR_LEN_MIN_IPV6ADDR;
	case SILC_MGMTD_VAR_DATETIME:
		return SILC_MGMTD_VAR_STR_LEN_MIN_DATETIME;
	case SILC_MGMTD_VAR_DATE:
		return SILC_MGMTD_VAR_STR_LEN_MIN_DATE;
	case SILC_MGMTD_VAR_TIME:
		return SILC_MGMTD_VAR_STR_LEN_MIN_TIME;
	case SILC_MGMTD_VAR_HEX:
		return p_var->val.hex_val.len*2+1;
	default:
		return SILC_MGMTD_VAR_STR_LEN_MIN_UNKNOWN;
	}
}

silc_cstr silc_mgmtd_var_uint32_to_str(uint32_t value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_32BIT);
	sprintf(str_buf, "%u", value);
	return str_buf;
}

silc_cstr silc_mgmtd_var_int32_to_str(int32_t value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_32BIT);
	sprintf(str_buf, "%d", value);
	return str_buf;
}

silc_cstr silc_mgmtd_var_uint64_to_str(uint64_t value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_64BIT);
	sprintf(str_buf, "%"PRIu64, value);
	return str_buf;
}

silc_cstr silc_mgmtd_var_int64_to_str(int64_t value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_64BIT);
	sprintf(str_buf, "%"PRId64, value);
	return str_buf;
}

silc_cstr silc_mgmtd_var_float_to_str(float value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_32BIT);
	sprintf(str_buf, "%f", value);
	return str_buf;
}

silc_cstr silc_mgmtd_var_double_to_str(double value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_64BIT);
	sprintf(str_buf, "%f", value);
	return str_buf;
}

silc_cstr silc_mgmtd_var_bool_to_str(silc_bool value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_BOOL);
	if(value)
		strcpy(str_buf, "true");
	else
		strcpy(str_buf, "false");
	return str_buf;
}
silc_cstr silc_mgmtd_var_boolean_to_str(silc_bool value, silc_cstr str_buf, int buf_len)
{
	return silc_mgmtd_var_bool_to_str(value, str_buf, buf_len);
}

silc_cstr silc_mgmtd_var_str_to_str(const silc_cstr value, silc_cstr str_buf, int buf_len)
{
	silc_cstr src_str = value;
	if(!value)
		src_str = "";

	silc_mgmd_var_convert_chk_len(buf_len, strlen(src_str)+1);
	strcpy(str_buf, src_str);
	return str_buf;
}

silc_cstr silc_mgmtd_var_strlines_to_str(const silc_cstr value, silc_cstr str_buf, int buf_len)
{
	silc_cstr src_str = value;
	if(!value)
		src_str = "";

	silc_mgmd_var_convert_chk_len(buf_len, silc_mgmtd_if_str_escape_len(src_str));
	silc_mgmtd_if_str_escape(src_str, str_buf);
	return str_buf;
}

silc_cstr silc_mgmtd_var_mac_addr_to_str(uint8_t* value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_MACADDR);
	sprintf(str_buf, "%02x:%02x:%02x:%02x:%02x:%02x",
			value[0], value[1],
			value[2], value[3],
			value[4], value[5]);
	return str_buf;
}

silc_cstr silc_mgmtd_var_ipv4_addr_to_str(uint32_t value, silc_cstr str_buf, int buf_len)
{
	struct in_addr in;
	in.s_addr = value;
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_IPV4ADDR);
	inet_ntop(AF_INET, &in, str_buf, buf_len);
	return str_buf;
}

silc_cstr silc_mgmtd_var_ipv6_addr_to_str(struct in6_addr* value, silc_cstr str_buf, int buf_len)
{
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_IPV6ADDR);
	inet_ntop(AF_INET6, value, str_buf, buf_len);
	return str_buf;
}

silc_cstr silc_mgmtd_var_ip_addr_to_str(silc_ipaddr* value, silc_cstr str_buf, int buf_len)
{
	if(value->af == AF_INET)
		return silc_mgmtd_var_ipv4_addr_to_str(value->addr.v4, str_buf, buf_len);
	else
		return silc_mgmtd_var_ipv6_addr_to_str(&(value->addr.v6), str_buf, buf_len);
	return str_buf;
}

silc_cstr silc_mgmtd_var_datetime_to_str(uint32_t value, silc_cstr str_buf, int buf_len)
{
	struct tm tm;
	time_t time = value;
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_DATETIME);
	localtime_r(&time, &tm);
	strftime(str_buf, buf_len, "%F,%T", &tm);
	return str_buf;
}

silc_cstr silc_mgmtd_var_date_to_str(uint32_t value, silc_cstr str_buf, int buf_len)
{
	struct tm tm;
	time_t time = value;
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_DATE);
	localtime_r(&time, &tm);
	strftime(str_buf, buf_len, "%F", &tm);
	return str_buf;
}

silc_cstr silc_mgmtd_var_time_to_str(uint32_t value, silc_cstr str_buf, int buf_len)
{
	struct tm tm;
	time_t time = value;
	silc_mgmd_var_convert_chk_len(buf_len, SILC_MGMTD_VAR_STR_LEN_MIN_TIME);
	localtime_r(&time, &tm);
	strftime(str_buf, buf_len, "%T", &tm);
	return str_buf;
}

silc_cstr silc_mgmtd_var_hex_to_str(silc_mgmtd_var_hex* value, silc_cstr str_buf, int buf_len)
{
	int i, len = value->len;

	silc_mgmd_var_convert_chk_len(buf_len, len*2+1);
	if(!value->data)
	{
		strcpy(str_buf, "");
		return str_buf;
	}
	for(i=0; i<len; i++)
		sprintf(str_buf+i*2, "%02x", value->data[i]);
	return str_buf;
}

silc_cstr silc_mgmtd_var_to_str(silc_mgmtd_var* p_var, silc_cstr str_buf, int buf_len)
{
	switch(p_var->type)
	{
	case SILC_MGMTD_VAR_NULL:
		strcpy(str_buf, "null");
		break;
	case SILC_MGMTD_VAR_UINT32:
		silc_mgmtd_var_uint32_to_str(p_var->val.uint32_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_INT32:
		silc_mgmtd_var_int32_to_str(p_var->val.int32_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_UINT64:
		silc_mgmtd_var_uint64_to_str(p_var->val.uint64_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_INT64:
		silc_mgmtd_var_int64_to_str(p_var->val.int64_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_FLOAT:
		silc_mgmtd_var_float_to_str(p_var->val.float_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_DOUBLE:
		silc_mgmtd_var_double_to_str(p_var->val.double_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_BOOL:
		silc_mgmtd_var_bool_to_str(p_var->val.bool_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_STRING:
		silc_mgmtd_var_str_to_str(p_var->val.string_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_STRING_LINES:
		silc_mgmtd_var_strlines_to_str(p_var->val.string_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_MACADDR:
		silc_mgmtd_var_mac_addr_to_str(p_var->val.macaddr_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_IPV4ADDR:
		silc_mgmtd_var_ipv4_addr_to_str(p_var->val.ipaddr_val.addr.v4, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_IPV6ADDR:
		silc_mgmtd_var_ipv6_addr_to_str(&(p_var->val.ipaddr_val.addr.v6), str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_IPADDR:
		silc_mgmtd_var_ip_addr_to_str(&(p_var->val.ipaddr_val), str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_DATETIME:
		silc_mgmtd_var_datetime_to_str(p_var->val.datetime_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_DATE:
		silc_mgmtd_var_date_to_str(p_var->val.date_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_TIME:
		silc_mgmtd_var_time_to_str(p_var->val.time_val, str_buf, buf_len);
		break;
	case SILC_MGMTD_VAR_HEX:
		silc_mgmtd_var_hex_to_str(&p_var->val.hex_val, str_buf, buf_len);
		break;
	default:
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_TYPE_UNKNOWN);
		return NULL;
	}
	return str_buf;
}


int silc_mgmtd_var_init(silc_mgmtd_var* p_var, silc_mgmtd_var_type type, const silc_cstr val_str)
{
	memset(p_var, 0, sizeof(*p_var));
	p_var->type = type;
	return silc_mgmtd_var_set_by_str(p_var, val_str);
}

void silc_mgmtd_var_clear(silc_mgmtd_var* p_var)
{
	if((p_var->type == SILC_MGMTD_VAR_STRING || p_var->type == SILC_MGMTD_VAR_STRING_LINES) &&
			p_var->val.string_val)
		free(p_var->val.string_val);
	if(p_var->type == SILC_MGMTD_VAR_HEX && p_var->val.hex_val.data)
		free(p_var->val.hex_val.data);
	memset(p_var, 0, sizeof(*p_var));
}

int silc_mgmtd_var_copy(silc_mgmtd_var* p_src_var, silc_mgmtd_var* p_dst_var)
{
	silc_mgmtd_var_clear(p_dst_var);

	p_dst_var->type = p_src_var->type;
	if((p_src_var->type == SILC_MGMTD_VAR_STRING || p_src_var->type == SILC_MGMTD_VAR_STRING_LINES) &&
			p_src_var->val.string_val)
	{
		p_dst_var->val.string_val = malloc(strlen(p_src_var->val.string_val)+1);
		if(!p_dst_var->val.string_val)
		{
			silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
			return -1;
		}
		strcpy(p_dst_var->val.string_val, p_src_var->val.string_val);
	}
	else if(p_src_var->type == SILC_MGMTD_VAR_HEX && p_src_var->val.hex_val.data)
	{
		p_dst_var->val.hex_val.data = malloc(p_src_var->val.hex_val.len);
		if(!p_dst_var->val.hex_val.data)
		{
			silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
			return -1;
		}
		memcpy(p_dst_var->val.hex_val.data, p_src_var->val.hex_val.data, p_src_var->val.hex_val.len);
		p_dst_var->val.hex_val.len = p_src_var->val.hex_val.len;
	}
	else
	{
		memcpy(p_dst_var, p_src_var, sizeof(*p_src_var));
	}

	return 0;
}

silc_bool silc_mgmtd_var_equal(silc_mgmtd_var* p_var0, silc_mgmtd_var* p_var1)
{
	if(p_var0->type != p_var1->type)
		return silc_false;

	switch(p_var0->type)
	{
	case SILC_MGMTD_VAR_UINT32:
		return (p_var0->val.uint32_val == p_var1->val.uint32_val);
	case SILC_MGMTD_VAR_INT32:
		return (p_var0->val.int32_val == p_var1->val.int32_val);
	case SILC_MGMTD_VAR_UINT64:
		return (p_var0->val.uint64_val == p_var1->val.uint64_val);
	case SILC_MGMTD_VAR_INT64:
		return (p_var0->val.int64_val == p_var1->val.int64_val);
	case SILC_MGMTD_VAR_FLOAT:
		return (p_var0->val.float_val == p_var1->val.float_val);
	case SILC_MGMTD_VAR_DOUBLE:
		return (p_var0->val.double_val == p_var1->val.double_val);
	case SILC_MGMTD_VAR_BOOL:
		return (p_var0->val.bool_val == p_var1->val.bool_val);
	case SILC_MGMTD_VAR_DATETIME:
		return (p_var0->val.datetime_val == p_var1->val.datetime_val);
	case SILC_MGMTD_VAR_DATE:
		return (p_var0->val.date_val == p_var1->val.date_val);
	case SILC_MGMTD_VAR_TIME:
		return (p_var0->val.time_val == p_var1->val.time_val);
	case SILC_MGMTD_VAR_IPV4ADDR:
		return (p_var0->val.ipaddr_val.addr.v4 == p_var1->val.ipaddr_val.addr.v4);
	case SILC_MGMTD_VAR_IPV6ADDR:
		return memcmp(&(p_var0->val.ipaddr_val.addr.v6), &(p_var1->val.ipaddr_val.addr.v6), sizeof(struct in6_addr));
	case SILC_MGMTD_VAR_IPADDR:
	{
		if(p_var0->val.ipaddr_val.af == AF_INET)
			return (p_var0->val.ipaddr_val.addr.v4 == p_var1->val.ipaddr_val.addr.v4);
		else
			return memcmp(&(p_var0->val.ipaddr_val.addr.v6), &(p_var1->val.ipaddr_val.addr.v6), sizeof(struct in6_addr));
	}
	case SILC_MGMTD_VAR_MACADDR:
	{
		int loop;
		for(loop=0; loop<6; loop++)
		{
			if(p_var0->val.macaddr_val[loop] != p_var1->val.macaddr_val[loop])
				return silc_false;
		}
		return silc_true;
	}
	case SILC_MGMTD_VAR_STRING:
	case SILC_MGMTD_VAR_STRING_LINES:
		if(!p_var0->val.string_val && !p_var1->val.string_val)
			return silc_true;
		else if(p_var0->val.string_val && p_var1->val.string_val)
		{
			if(strcmp(p_var0->val.string_val, p_var1->val.string_val) == 0)
				return silc_true;
		}
		return silc_false;
	case SILC_MGMTD_VAR_HEX:
	{
		int i;
		if(!p_var0->val.hex_val.data && !p_var1->val.hex_val.data)
			return silc_true;
		else if(p_var0->val.hex_val.len != p_var1->val.hex_val.len)
			return silc_false;
		else if(p_var0->val.hex_val.data && p_var1->val.hex_val.data)
		{
			for(i=0; i<p_var0->val.hex_val.len; i++)
			{
				if(p_var0->val.hex_val.data[i] != p_var1->val.hex_val.data[i])
					return silc_false;
			}
			return silc_true;
		}
		return silc_false;
	}
	default:
		return silc_false;
	}
	return silc_false;
}


/*
 *  value string to variables value
 */
static int silc_mgmtd_var_str2ll(const silc_cstr val_str, int64_t *p_val)
{
	long long result;
	char *endptr = NULL;

	result = strtoll(val_str, &endptr, 0);
	if(*endptr != '\0')
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}
	*p_val = result;
	return 0;
}

static int silc_mgmtd_var_str2l(const silc_cstr val_str, int32_t *p_val)
{
	long int result;
	char *endptr = NULL;

	result = strtol(val_str, &endptr, 0);
	if(*endptr != '\0')
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}
//	printf("[%s] val_str:%s, val:%lu\n", __func__, val_str, result);
	*p_val = result;
	return 0;
}


static int silc_mgmtd_var_str2f(const silc_cstr val_str, float *p_val)
{
	float result;
	char *endptr = NULL;

	result = strtof(val_str, &endptr);

	if(endptr == val_str || *endptr != '\0')
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}

	if(result == 0 && errno == ERANGE)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_VAL_OUTOFRANGE);
		return -1;
	}

	*p_val = result;
	return 0;
}

static int silc_mgmtd_var_str2d(const silc_cstr val_str, double *p_val)
{
	double result;
	char *endptr = NULL;

	result = strtod(val_str, &endptr);

	if(endptr == val_str || *endptr != '\0')
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}

	if(result == 0 && errno == ERANGE)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_VAL_OUTOFRANGE);
		return -1;
	}

	*p_val = result;
	return 0;
}

static int silc_mgmtd_var_str2bool(const silc_cstr val_str, silc_bool *p_val)
{
	int loop;
	silc_bool result;

	silc_cstr true_strs[] = { 	"active",
								"enable",
								"up",
								"yes",
								"inused",
								"true",
								"on",
								"1"
						};
	silc_cstr false_strs[] = { "suspended",
								"disable",
								"inactive",
								"down",
								"no",
								"free",
								"false",
								"off",
								"0"
						};

	for(loop=0; loop<sizeof(true_strs)/sizeof(silc_cstr); loop++)
	{
		if(strcmp(true_strs[loop], val_str) == 0)
		{
			result = silc_true;
			goto OUT;
		}
	}

	for(loop=0; loop<sizeof(false_strs)/sizeof(silc_cstr); loop++)
	{
		if(strcmp(false_strs[loop], val_str) == 0)
		{
			result = silc_false;
			goto OUT;
		}
	}
	silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
	return -1;
OUT:
	*p_val = result;
	return 0;
}

static int silc_mgmtd_var_str2str(const silc_cstr val_str, silc_cstr *p_val)
{
	int len = strlen(val_str);
	silc_cstr result = malloc(len+1);

	if(!result)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}
	if (val_str[0] == '\'' && val_str[len-1] == '\'')
	{
		strncpy(result, val_str+1, len-2);
		result[len-2] = 0;
	}
	else
		strcpy(result, val_str);
	if(*p_val)
		free(*p_val);
	*p_val = result;
	return 0;
}

static int silc_mgmtd_var_str2strlines(const silc_cstr val_str, silc_cstr *p_val)
{
	silc_cstr tmp = NULL;
	int len;
	silc_cstr result;

	if(0 != silc_mgmtd_var_str2str(val_str, &tmp))
		return -1;
	len = strlen(tmp);
	result = malloc(len+1);
	if(!result)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}
	silc_mgmtd_if_str_unescape(tmp, result);
	if(*p_val)
		free(*p_val);
	*p_val = result;
	return 0;
}

static int silc_mgmtd_var_str2ipv4addr(const silc_cstr val_str, uint32_t *p_val)
{
	struct in_addr addr;
	int result;

	if(strcmp(val_str, "") == 0)
	{
		*p_val = 0;
		return 0;
	}

	result = inet_pton(AF_INET, val_str, &addr);
	if(result != 1)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}

	*p_val = *(uint32_t*)(&addr);
	return 0;
}

static int silc_mgmtd_var_str2ipv6addr(const silc_cstr val_str, struct in6_addr *p_val)
{
	int result;

	if(strcmp(val_str, "") == 0)
	{
		memset(p_val, 0, sizeof(struct in6_addr));
		return 0;
	}

	result = inet_pton(AF_INET6, val_str, p_val);
	if(result != 1)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}

	return 0;
}

static int silc_mgmtd_var_str2ipaddr(const silc_cstr val_str, silc_ipaddr *p_val)
{
	int af, ret;
	if(strstr(val_str, ":"))
	{
		af = AF_INET6;
		ret = silc_mgmtd_var_str2ipv6addr(val_str, &(p_val->addr.v6));
	}
	else
	{
		af = AF_INET;
		ret = silc_mgmtd_var_str2ipv4addr(val_str, &(p_val->addr.v4));
	}
	if(ret)
		return -1;

	p_val->af = af;
	return 0;
}

static int silc_mgmtd_var_str2macaddr(const silc_cstr val_str, uint8_t *p_val)
{
	int ret, loop;
	unsigned int addr[6];

	ret = sscanf(val_str, "%x:%x:%x:%x:%x:%x",
			&addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]);
	if(ret != 6)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}

	for(loop=0; loop<6; loop++)
	{
		if(addr[loop] < 0 || addr[loop] > 0xff)
		{
			silc_mgmtd_lib_err_set(LIB_ERR_VAR_VAL_OUTOFRANGE);
			return -1;
		}
		*(p_val+loop) = (uint8_t)addr[loop];
	}
	return 0;
}

static int silc_mgmtd_var_str2hex(const silc_cstr val_str, silc_mgmtd_var_hex *p_val)
{
	int ret, loop, vlen, slen;;
	unsigned int hex;
	char tmp[10];
	uint8_t* p_data;

	slen = strlen(val_str);
	if(slen == 0 || strcmp(val_str, "None") == 0)
	{
		p_val->len = 0;
		p_val->data = NULL;
		return 0;
	}
	else if (slen&0x1) // hex str len should be 2N
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}

	vlen = slen/2;
	p_data = malloc(vlen);
	if(!p_data)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}

	for(loop=0; loop<vlen; loop++)
	{
		strncpy(tmp, val_str+loop*2, 2);
		tmp[2] = 0;
		ret = sscanf(tmp, "%x", &hex);
		if(ret != 1)
		{
			free(p_data);
			silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
			return -1;
		}
		p_data[loop] = hex;
	}

	p_val->len = vlen;
	if(p_val->data)
		free(p_val->data);
	p_val->data = p_data;
	return 0;
}

static int silc_mgmtd_var_str2time(const silc_cstr val_str, uint32_t *p_val)
{
	struct tm tm, diff_tm;
	time_t time;
	time_t utc_time = 24*3600;
	int diff_time;

	memset(&tm, 0, sizeof(tm));
	if(!strptime(val_str, "%T", &tm))
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}
	time = tm.tm_hour*3600 + tm.tm_min*60 + tm.tm_sec;
	localtime_r(&utc_time, &diff_tm);
	// use diff time to calculate the local time diff
	diff_time = (diff_tm.tm_hour*3600 + diff_tm.tm_min*60 + diff_tm.tm_sec)-utc_time;
	*p_val = (time - diff_time + 24*3600)%(24*3600);
	return 0;
}

static int silc_mgmtd_var_str2date(const silc_cstr val_str, uint32_t *p_val)
{
	struct tm tm;
	time_t time;
	//struct tm diff_tm;
	//time_t utc_time = 24*3600;
	//int diff_time;

	memset(&tm, 0, sizeof(tm));
	if(!strptime(val_str, "%Y-%m-%d", &tm))
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}
	time = mktime(&tm);
	//localtime_r(&utc_time, &diff_tm);
	// use diff time to calculate the local time diff
	//diff_time = (diff_tm.tm_hour*3600 + diff_tm.tm_min*60 + diff_tm.tm_sec)-utc_time;
	//*p_val = time - diff_time;
	*p_val = time;
	return 0;
}

static int silc_mgmtd_var_str2datetime(const silc_cstr val_str, uint32_t *p_val)
{
	silc_cstr_array* p_str_arr;
	silc_cstr date_str;
	silc_cstr time_str;
	uint32_t date_n, time_n;

	p_str_arr = silc_cstr_split(val_str, ",");
	if(!p_str_arr)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_VAL_BUFF_NOT_ENOUGH);
		return -1;
	}
	date_str = silc_cstr_array_get_quick(p_str_arr, 0);
	time_str = silc_cstr_array_get_quick(p_str_arr, 1);
	if(!date_str || !time_str)
	{
		silc_cstr_array_destroy(p_str_arr);
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
		return -1;
	}

	if(silc_mgmtd_var_str2date(date_str, &date_n) < 0 ||
			silc_mgmtd_var_str2time(time_str, &time_n) < 0)
	{
		silc_cstr_array_destroy(p_str_arr);
		return -1;
	}
	silc_cstr_array_destroy(p_str_arr);

	*p_val = date_n + time_n;
	return 0;
}


/*
 * Set variables value by value string
 */
int silc_mgmtd_var_set_by_str(silc_mgmtd_var* p_var, const silc_cstr val_str)
{
	int ret;

	switch(p_var->type)
	{
	case SILC_MGMTD_VAR_NULL:
		ret = 0;
		break;
	case SILC_MGMTD_VAR_UINT32:
		if(val_str[0] == '-')
		{
			silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
                        return -1;
		}
	case SILC_MGMTD_VAR_INT32:
		ret= silc_mgmtd_var_str2l(val_str, &p_var->val.int32_val);
		break;
	case SILC_MGMTD_VAR_UINT64:
                if(val_str[0] == '-')
                {
                        silc_mgmtd_lib_err_set(LIB_ERR_VAR_INVALID_VAL_STR);
                        return -1;
                }
	case SILC_MGMTD_VAR_INT64:
		ret= silc_mgmtd_var_str2ll(val_str, &p_var->val.int64_val);
		break;
	case SILC_MGMTD_VAR_FLOAT:
		ret = silc_mgmtd_var_str2f(val_str, &p_var->val.float_val);
		break;
	case SILC_MGMTD_VAR_DOUBLE:
		ret = silc_mgmtd_var_str2d(val_str, &p_var->val.double_val);
		break;
	case SILC_MGMTD_VAR_BOOL:
		ret = silc_mgmtd_var_str2bool(val_str, &p_var->val.bool_val);
		break;
	case SILC_MGMTD_VAR_STRING:
		ret = silc_mgmtd_var_str2str(val_str, &p_var->val.string_val);
		break;
	case SILC_MGMTD_VAR_STRING_LINES:
		ret = silc_mgmtd_var_str2strlines(val_str, &p_var->val.string_val);
		break;
	case SILC_MGMTD_VAR_MACADDR:
		ret = silc_mgmtd_var_str2macaddr(val_str, &p_var->val.macaddr_val[0]);
		break;
	case SILC_MGMTD_VAR_IPV4ADDR:
		ret = silc_mgmtd_var_str2ipv4addr(val_str, &(p_var->val.ipaddr_val.addr.v4));
		break;
	case SILC_MGMTD_VAR_IPV6ADDR:
		ret = silc_mgmtd_var_str2ipv6addr(val_str, &(p_var->val.ipaddr_val.addr.v6));
		break;
	case SILC_MGMTD_VAR_IPADDR:
		ret = silc_mgmtd_var_str2ipaddr(val_str, &(p_var->val.ipaddr_val));
		break;
	case SILC_MGMTD_VAR_DATETIME:
		ret = silc_mgmtd_var_str2datetime(val_str, &p_var->val.datetime_val);
		break;
	case SILC_MGMTD_VAR_DATE:
		ret = silc_mgmtd_var_str2date(val_str, &p_var->val.date_val);
		break;
	case SILC_MGMTD_VAR_TIME:
		ret = silc_mgmtd_var_str2time(val_str, &p_var->val.time_val);
		break;
	case SILC_MGMTD_VAR_HEX:
		ret = silc_mgmtd_var_str2hex(val_str, &p_var->val.hex_val);
		break;
	default:
	{
		silc_mgmtd_lib_err_set(LIB_ERR_VAR_TYPE_UNKNOWN);
		return -1;
	}
	}
	return ret;
}

