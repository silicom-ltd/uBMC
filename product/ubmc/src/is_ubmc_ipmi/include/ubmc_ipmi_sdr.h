
#ifndef _UBMC_IPMI_SDR_H
#define _UBMC_IPMI_SDR_H


#define tos32(val, bits)    ((val & ((1<<((bits)-1)))) ? (-((val) & (1<<((bits)-1))) | (val)) : (val))

//add by jamesxie 20180504 , The values of bacc are derived from these three values --- (b,k1,k2).
#define rev_tos32(val,bits)  (((val) < 0 ) ? (1<<(bits)) + (val) : (val))

# define BSWAP_16(x) ((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))
# define BSWAP_32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) |\
                     (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))

# define __TO_TOL(mtol)     (uint16_t)(BSWAP_16(mtol) & 0x3f)
# define __TO_M(mtol)       (int16_t)(tos32((((BSWAP_16(mtol) & 0xff00) >> 8) | ((BSWAP_16(mtol) & 0xc0) << 2)), 10))
# define __TO_B(bacc)       (int32_t)(tos32((((BSWAP_32(bacc) & 0xff000000) >> 24) | \
                            ((BSWAP_32(bacc) & 0xc00000) >> 14)), 10))
# define __TO_ACC(bacc)     (uint32_t)(((BSWAP_32(bacc) & 0x3f0000) >> 16) | ((BSWAP_32(bacc) & 0xf000) >> 6))
# define __TO_ACC_EXP(bacc) (uint32_t)((BSWAP_32(bacc) & 0xc00) >> 10)
# define __TO_R_EXP(bacc)   (int32_t)(tos32(((BSWAP_32(bacc) & 0xf0) >> 4), 4))
# define __TO_B_EXP(bacc)   (int32_t)(tos32((BSWAP_32(bacc) & 0xf), 4))


//add by jamesxie 20180504 , The values of bacc are derived from these three values --- (b,k1,k2).
# define __REV_TO_B(r)          (int32_t)((rev_tos32(r,10) & 0xff) << 24)
# define __REV_TO_R_EXP(r)      (int32_t)((rev_tos32(r,4) & 0x0f) << 4)
# define __REV_TO_B_EXP(r)      (int32_t)(rev_tos32(r,4) & 0x0f)

# define __REV_TO_BACC(b,k1,k2) (int32_t)( BSWAP_32(__REV_TO_B(b) | __REV_TO_R_EXP(k2) | \
															__REV_TO_B_EXP(k1)))


#pragma pack(1)

struct entity_id {
	uint8_t	id;			/* physical entity id */
	uint8_t	instance    : 7;	/* instance number */
	uint8_t	logical     : 1;	/* physical/logical */
} ;

#pragma pack(0)


#pragma pack(1)

struct sdr_repo_info_rs {
	uint8_t version;	/* SDR version (51h) */
	uint16_t count;		/* number of records */
	uint16_t free;		/* free space in SDR */
	uint32_t add_stamp;	/* last add timestamp */
	uint32_t erase_stamp;	/* last del timestamp */
	uint8_t op_support;	/* supported operations */
} ;

#pragma pack(0)


#pragma pack(1)

/* builtin (device) sdrs support */
struct sdr_device_info_rs {
	unsigned char count;	/* number of records */
	unsigned char flags;	/* flags */
	unsigned char popChangeInd[3];	/* free space in SDR */
} ;

#pragma pack(0)

#pragma pack(1)

#define GET_SDR_RESERVE_REPO	0x22
struct sdr_reserve_repo_rs {
	uint16_t reserve_id;	/* reservation ID */
} ;

#pragma pack(0)




#pragma pack(1)

#define GET_SDR		0x23
struct sdr_get_rq {
	uint16_t reserve_id;	/* reservation ID */
	uint16_t id;		/* record ID */
	uint8_t offset;		/* offset into SDR */
#define GET_SDR_ENTIRE_RECORD	0xff
	uint8_t length;		/* length to read */
} ;

#pragma pack(0)



#pragma pack(1)

struct sdr_get_rs {
	uint16_t next;		/* next record id */
	uint16_t id;		/* record ID */
	uint8_t version;	/* SDR version (51h) */
#define SDR_RECORD_TYPE_FULL_SENSOR		0x01
#define SDR_RECORD_TYPE_COMPACT_SENSOR		0x02
#define SDR_RECORD_TYPE_EVENTONLY_SENSOR	0x03
#define SDR_RECORD_TYPE_ENTITY_ASSOC		0x08
#define SDR_RECORD_TYPE_DEVICE_ENTITY_ASSOC	0x09
#define SDR_RECORD_TYPE_GENERIC_DEVICE_LOCATOR	0x10
#define SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR	0x11
#define SDR_RECORD_TYPE_MC_DEVICE_LOCATOR	0x12
#define SDR_RECORD_TYPE_MC_CONFIRMATION		0x13
#define SDR_RECORD_TYPE_BMC_MSG_CHANNEL_INFO	0x14
#define SDR_RECORD_TYPE_OEM			0xc0
	uint8_t type;		/* record type */
	uint8_t length;		/* remaining record bytes */
} ;

#pragma pack(0)


struct sdr_record_mask {
	union {
		struct {
			uint16_t assert_event;	/* assertion event mask */
			uint16_t deassert_event;	/* de-assertion event mask */
			uint16_t read;	/* discrete reading mask */
		}  discrete;
		struct {

			uint16_t assert_lnc_low:1;
			uint16_t assert_lnc_high:1;
			uint16_t assert_lcr_low:1;
			uint16_t assert_lcr_high:1;
			uint16_t assert_lnr_low:1;
			uint16_t assert_lnr_high:1;
			uint16_t assert_unc_low:1;
			uint16_t assert_unc_high:1;
			uint16_t assert_ucr_low:1;
			uint16_t assert_ucr_high:1;
			uint16_t assert_unr_low:1;
			uint16_t assert_unr_high:1;
			uint16_t status_lnc:1;
			uint16_t status_lcr:1;
			uint16_t status_lnr:1;
			uint16_t reserved:1;


			uint16_t deassert_lnc_low:1;
			uint16_t deassert_lnc_high:1;
			uint16_t deassert_lcr_low:1;
			uint16_t deassert_lcr_high:1;
			uint16_t deassert_lnr_low:1;
			uint16_t deassert_lnr_high:1;
			uint16_t deassert_unc_low:1;
			uint16_t deassert_unc_high:1;
			uint16_t deassert_ucr_low:1;
			uint16_t deassert_ucr_high:1;
			uint16_t deassert_unr_low:1;
			uint16_t deassert_unr_high:1;
			uint16_t status_unc:1;
			uint16_t status_ucr:1;
			uint16_t status_unr:1;
			uint16_t reserved_2:1;
			union {
				struct {

					uint16_t readable:8;
					uint16_t lnc:1;
					uint16_t lcr:1;
					uint16_t lnr:1;
					uint16_t unc:1;
					uint16_t ucr:1;
					uint16_t unr:1;
					uint16_t reserved:2;

				}  set;
				struct {

					uint16_t lnc:1;
					uint16_t lcr:1;
					uint16_t lnr:1;
					uint16_t unc:1;
					uint16_t ucr:1;
					uint16_t unr:1;
					uint16_t reserved:2;
					uint16_t settable:8;

				}  read;
			} ;
		}  threshold;
	}  type;
} ;

#pragma pack(0)



#pragma pack(1)

struct sdr_record_common_sensor {
	struct {
		uint8_t owner_id;

		uint8_t lun:2;	/* sensor owner lun */
		uint8_t __reserved:2;
		uint8_t channel:4;	/* channel number */

		uint8_t sensor_num;	/* unique sensor number */
	}  keys;

	struct entity_id entity;

	struct {
		struct {

			uint8_t sensor_scan:1;
			uint8_t event_gen:1;
			uint8_t type:1;
			uint8_t hysteresis:1;
			uint8_t thresholds:1;
			uint8_t events:1;
			uint8_t scanning:1;
			uint8_t __reserved:1;

		}  init;
		struct {

			uint8_t event_msg:2;
			uint8_t threshold:2;
			uint8_t hysteresis:2;
			uint8_t rearm:1;
			uint8_t ignore:1;

		}  capabilities;
		uint8_t type;
	}  sensor;

	uint8_t event_type;	/* event/reading type code */

	struct sdr_record_mask mask;

	struct {

		uint8_t pct:1;
		uint8_t modifier:2;
		uint8_t rate:3;
		uint8_t analog:2;

		struct {
			uint8_t base;
			uint8_t modifier;
		}  type;
	}  unit;
} ;

/* SDR Record Common Sensor header macros */
#define IS_THRESHOLD_SENSOR(s)	((s)->event_type == 1)
#define UNITS_ARE_DISCRETE(s)	((s)->unit.analog == 3)


#pragma pack(0)



#pragma pack(1)

struct sdr_record_compact_sensor {
	struct sdr_record_common_sensor cmn;
	struct {

		uint8_t count:4;
		uint8_t mod_type:2;
		uint8_t __reserved:2;


		uint8_t mod_offset:7;
		uint8_t entity_inst:1;

	}  share;

	struct {
		struct {
			uint8_t positive;
			uint8_t negative;
		}  hysteresis;
	}  threshold;

	uint8_t __reserved[3];
	uint8_t oem;		/* reserved for OEM use */
	uint8_t id_code;	/* sensor ID string type/length code */
	uint8_t id_string[16];	/* sensor ID string bytes, only if id_code != 0 */
} ;

#pragma pack(0)


#pragma pack(1)

struct sdr_record_eventonly_sensor {
	struct {
		uint8_t owner_id;

		uint8_t lun:2;	/* sensor owner lun */
		uint8_t fru_owner:2;	/* fru device owner lun */
		uint8_t channel:4;	/* channel number */

		uint8_t sensor_num;	/* unique sensor number */
	}  keys;

	struct entity_id entity;

	uint8_t sensor_type;	/* sensor type */
	uint8_t event_type;	/* event/reading type code */

	struct {

		uint8_t count:4;
		uint8_t mod_type:2;
		uint8_t __reserved:2;

		uint8_t mod_offset:7;
		uint8_t entity_inst:1;

	}  share;

	uint8_t __reserved;
	uint8_t oem;		/* reserved for OEM use */
	uint8_t id_code;	/* sensor ID string type/length code */
	uint8_t id_string[16];	/* sensor ID string bytes, only if id_code != 0 */

} ;

#pragma pack(0)



#pragma pack(1)

struct sdr_record_full_sensor {
	struct sdr_record_common_sensor cmn;

#define SDR_SENSOR_L_LINEAR     0x00
#define SDR_SENSOR_L_LN         0x01
#define SDR_SENSOR_L_LOG10      0x02
#define SDR_SENSOR_L_LOG2       0x03
#define SDR_SENSOR_L_E          0x04
#define SDR_SENSOR_L_EXP10      0x05
#define SDR_SENSOR_L_EXP2       0x06
#define SDR_SENSOR_L_1_X        0x07
#define SDR_SENSOR_L_SQR        0x08
#define SDR_SENSOR_L_CUBE       0x09
#define SDR_SENSOR_L_SQRT       0x0a
#define SDR_SENSOR_L_CUBERT     0x0b
#define SDR_SENSOR_L_NONLINEAR  0x70

	uint8_t linearization;	/* 70h=non linear, 71h-7Fh=non linear, OEM */
	uint16_t mtol;		/* M, tolerance */
	uint32_t bacc;		/* accuracy, B, Bexp, Rexp */

	struct {

		uint8_t nominal_read:1;	/* nominal reading field specified */
		uint8_t normal_max:1;	/* normal max field specified */
		uint8_t normal_min:1;	/* normal min field specified */
		uint8_t __reserved:5;

	}  analog_flag;

	uint8_t nominal_read;	/* nominal reading, raw value */
	uint8_t normal_max;	/* normal maximum, raw value */
	uint8_t normal_min;	/* normal minimum, raw value */
	uint8_t sensor_max;	/* sensor maximum, raw value */
	uint8_t sensor_min;	/* sensor minimum, raw value */

	struct {
		struct {
			uint8_t non_recover;
			uint8_t critical;
			uint8_t non_critical;
		}  upper;
		struct {
			uint8_t non_recover;
			uint8_t critical;
			uint8_t non_critical;
		}  lower;
		struct {
			uint8_t positive;
			uint8_t negative;
		}  hysteresis;
	}  threshold;
	uint8_t __reserved[2];
	uint8_t oem;		/* reserved for OEM use */
	uint8_t id_code;	/* sensor ID string type/length code */
	uint8_t id_string[16];	/* sensor ID string bytes, only if id_code != 0 */
} ;

#pragma pack(0)


#endif				/* IPMI_SDR_H */
