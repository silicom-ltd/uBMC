#ifndef BIN_UBMC_COMMON_H_
#define BIN_UBMC_COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>


#define HB_SW_STAT
#define BCM_LINKSCAN_ENABLE
#define FIX_CPLD_RESET_WDT_EXP
#define ENABLE_RX_ERR_STAT
#define ENABLE_TX_ERR_STAT
#define PROCESS_RX_TX_ERROR_STATISTICS
#define FIX_CPLD_RESET_WDT_EXP
#define __BCM_40G_CONFIG
#define DISABLE_PNP
#define ENABLE_ATERNATIVE_UPDATE_ONLY_FROM_BADAS
#define DISABLE_CHECK_CPU_OVERLOAD

#ifndef IS_USE_PRINTF

#define is_log(lvl, fmt, ...)	syslog(lvl, fmt, ## __VA_ARGS__)
#define is_log_init(fmt, ...)	syslog(LOG_INFO, fmt, ## __VA_ARGS__)
#define is_debug(fmt, ...)		syslog(LOG_DEBUG, fmt, ## __VA_ARGS__)
#define	is_info(fmt, ...)		syslog(LOG_INFO, fmt, ## __VA_ARGS__)
#define is_notice(fmt, ...)		syslog(LOG_NOTICE, fmt, ## __VA_ARGS__)
#define is_warn(fmt, ...)		syslog(LOG_WARNING, fmt, ## __VA_ARGS__)
#define is_err(fmt, ...)		syslog(LOG_ERR, fmt, ## __VA_ARGS__)
#define is_err_sys(fmt, ...)	syslog(LOG_ERR, fmt" Error %s:%d", ## __VA_ARGS__, strerror(errno), errno)

#include <syslog.h>

#else

#define is_log(lvl, fmt, ...)	fprintf(stderr, fmt, ## __VA_ARGS__)
#define is_log_init(fmt, ...)	fprintf(stderr, "[INIT]"fmt"\n", ## __VA_ARGS__)
#define	is_debug(fmt, ...)		fprintf(stderr, "[DBG ]"fmt"\n", ## __VA_ARGS__)
#define	is_info(fmt, ...)		fprintf(stderr, "[INFO]"fmt"\n", ## __VA_ARGS__)
#define is_notice(fmt, ...)		fprintf(stderr, "[NOTI]"fmt"\n", ## __VA_ARGS__)
#define is_warn(fmt, ...)		fprintf(stderr, "[WARN]"fmt"\n", ## __VA_ARGS__)
#define is_err(fmt, ...)		fprintf(stderr, "[ERR ]"fmt"\n", ## __VA_ARGS__)
#define is_err_sys(fmt, ...)	fprintf(stderr, "[ERR ]"fmt" Error %s:%d\n", ## __VA_ARGS__, strerror(errno), errno)

#endif


#define is_err_once(fmt, ...)	\
	do { \
		static int __func__##__line__##once = 1; \
		if(__func__##__line__##once) {	\
			is_err(fmt, ## __VA_ARGS__); \
			__func__##__line__##once = 0;\
		}\
	}while(0)

#define is_print_stats(fmt, ...)	fprintf(stdout, fmt, ## __VA_ARGS__)
#define NUM_VA_ARGS(...)  ((sizeof((int[]){0, ## __VA_ARGS__})/sizeof(int))-1)

#define PP_NARG(...) \
		(PP_NARG_(__VA_ARGS__,PP_RSEQ_N()))
#define PP_NARG_(...) \
		PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
		_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
		_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
		_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
		_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
		_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
		_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
		_61,_62,_63,N,...) N
#define PP_RSEQ_N() \
		63,62,61,60,                   \
		59,58,57,56,55,54,53,52,51,50, \
		49,48,47,46,45,44,43,42,41,40, \
		39,38,37,36,35,34,33,32,31,30, \
		29,28,27,26,25,24,23,22,21,20, \
		19,18,17,16,15,14,13,12,11,10, \
		9,8,7,6,5,4,3,2,1,0

static inline void __is_print_func_arg(char* prefix, int cnt, ...)
{
	uintptr_t arg;
	uint32_t loop;
	va_list vl;

	va_start(vl, cnt);

	fprintf(stderr, "%s ", prefix);
	for(loop = 0; loop < cnt; loop++)
	{
		arg=va_arg(vl,uintptr_t);
		fprintf(stderr, "arg%u:%08"PRIxPTR" ", loop, arg);
	}
	fprintf(stderr, "\n");
	va_end(vl);
}
#if 0
#define is_print_func_arg(prefix, ...)	\
	__is_print_func_arg(prefix, PP_NARG(__VA_ARGS__), ## __VA_ARGS__);
#else
#define is_print_func_arg(prefix, ...)	do{}while(0)
#endif
#define is_call_init(func, ...)	\
	do { \
		ret = func(__VA_ARGS__);	\
		if(ret<0){ \
			is_err("Failed to initialize %s", #func); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
			goto ERR_RET; \
		} \
	}while(0)

#define is_call_create(ret_obj, func, ...) \
	do { \
		ret_obj = func(__VA_ARGS__);	\
		if(ret_obj==NULL){ \
			ret = -1; \
			is_err("Failed to create %s", #func); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
			if(ret!=0) goto ERR_RET; \
		} \
	}while(0)

#define is_call_alloc(ret_obj, func, ...) \
	do { \
		ret_obj = func(__VA_ARGS__);	\
		if(ret_obj==NULL){ \
			ret = -1; \
			is_err("Failed to allocate %s", #func); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
			goto ERR_RET; \
		} \
	}while(0)

#define is_check_err_ret(condition, _err_number, fmt, ...)\
	if(condition) { \
		is_err(fmt, ## __VA_ARGS__); \
		errno = _err_number; \
		return -1; \
	}

#define is_call_func(func, ...)	\
	do { \
		ret = func(__VA_ARGS__);	\
		if(ret<0){ \
			is_err("Failed to call %s", #func); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
			goto ERR_RET; \
		} \
	}while(0)
#define is_call_func_no_ret(func, ...)	\
	do { \
		ret = func(__VA_ARGS__);	\
		if(ret<0){ \
			is_err("Failed to call %s", #func); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
		} \
	}while(0)


#define is_call_bcm(func, ...)	\
	do { \
		ret = func(__VA_ARGS__);	\
		if(ret<0){ \
			is_err("Failed to call bcm %s error %s", #func, bcm_errmsg(ret)); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
			goto ERR_RET; \
		} \
	}while(0)

#define is_call_bcm_break(func, ...)	\
		ret = func(__VA_ARGS__);	\
		if(ret<0){ \
			is_err("Failed to bcm %s error %s", #func, bcm_errmsg(ret)); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
			break;\
		} \

#define is_call_bcm_noret(func, ...)	\
	do { \
		ret = func(__VA_ARGS__);	\
		if(ret<0){ \
			is_err("Failed to bcm %s error %s", #func, bcm_errmsg(ret)); \
			is_print_func_arg("\t"#func, ## __VA_ARGS__); \
		} \
	}while(0)

#define is_config_dbg(fmt, ...)	\
	fprintf(stderr, fmt"\n", ## __VA_ARGS__)

typedef struct is_segment_id_s
{
	union {
		struct {
			uint8_t		unit_id;
			uint8_t		module_id;
			uint16_t	segment_id;
		}s;
		uint32_t	id;
	}u;
}is_segment_id;

typedef struct {
	int counter;
} is_atomic;


#define IS_UI_ID(id)			((id)+1)
#define IS_UI_MODULE_ID(seg_id)		((seg_id).u.s.module_id + 1)
#define IS_UI_SEGMENT_ID(seg_id)	((seg_id).u.s.segment_id +1)

#define IS_MODULE_ID_FROM_UI_ID(ui_id)	(ui_id==0?IS_LIMIT_MODULES:(ui_id)-1)
#define IS_SEGMENT_ID_FROM_UI_ID(ui_id)	(ui_id==0?IS_LIMIT_MODULE_SEGMENTS:(ui_id)-1)
#define IS_PORT_ID_FROM_UI_ID(ui_id)	(ui_id==0?IS_LIMIT_SEGMENT_PORTS:(ui_id)-1)
#define IS_FAN_ID_FROM_UI_ID(ui_id)	((ui_id)-1)


#endif /* BIN_IS_COMMON_H_ */
