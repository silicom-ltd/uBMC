#ifndef __EEPROM_OP_H_
#define __EEPROM_OP_H_
#include <syslog.h>
#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int

#define EEPROM_SIZE          (256)
#define ID_STRING_LEN        (8)
#define VENDOR_NAME_LEN      (7)
#define PRODUCT_NAME_LEN     (8)
#define PART_NUM_LEN         (14)
#define SERIAL_NUM_LEN       (12)
#define LABEL_REVISION_LEN   (4)
#define MAC_1_BASE_LEN       (6)
#define MANUFACTURE_DATE_LEN (19)
#define NUM_MACS_LEN         (2)
#define MANUFACTURE_LEN      (7)
#define COUNTRY_CODE_LEN     (2)
#define VENDOR_EXTENSION_LEN (46)
#define CRC_32_LEN           (4)

/*
 *  Without getting too philosophical, define truth, falsehood, and the
 *  boolean type, if they are not already defined.
 */
#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#define eeprom_debug(fmt, ...) \
            syslog(LOG_NOTICE, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__);
#define eeprom_err(fmt, ...) \
            syslog(LOG_ERR, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__);
/*
 * TlvInfo header: Layout of the header for the TlvInfo format
 *
 * See the end of this file for details of this eeprom format
 */
struct __attribute__ ((__packed__)) tlvinfo_header_s
{
	char signature[8]; /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
	u8 version; /* 0x08        Structure version    */
	u16 totallen; /* 0x09 - 0x0A Length of all data which follows */
};
typedef struct tlvinfo_header_s tlvinfo_header_t;

// Header Field Constants
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_INFO_MAX_LEN        2048
#define TLV_TOTAL_LEN_MAX       (TLV_INFO_MAX_LEN - sizeof(tlvinfo_header_t))

#define min(x,y) (x < y ? x : y)
/*
 * TlvInfo TLV: Layout of a TLV field
 */
struct __attribute__ ((__packed__)) tlvinfo_tlv_s
{
	u8 type;
	u8 length;
	u8 value[0];
};
typedef struct tlvinfo_tlv_s tlvinfo_tlv_t;

/* Maximum length of a TLV value in bytes */
#define TLV_VALUE_MAX_LEN        255

/*  The TLV Types.
 *
 *  Keep these in sync with tlv_code_list in cmd_sys_eeprom.c
 */
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_DIAG_VERSION   0x2E
#define TLV_CODE_SERVICE_TAG    0x2F
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE

/*
 *the conversion of little endian and big endian
 */
#define be16_to_cpu(data) ((data & 0xff) <<8 |(data >> 8) & 0xff)
#define cpu_to_be16(data) ((data >> 8) & 0xff | (data & 0xff) << 8)

#define is_digit(c)             ((c) >= '0' && (c) <= '9')

/**
 *  is_valid_tlvinfo_header
 *
 *  Perform sanity checks on the first 11 bytes of the TlvInfo EEPROM
 *  data pointed to by the parameter:
 *      1. First 8 bytes contain null-terminated ASCII string "TlvInfo"
 *      2. Version byte is 1
 *      3. Total length bytes contain value which is less than or equal
 *         to the allowed maximum (2048-11)
 *
 */
static inline bool is_valid_tlvinfo_header(tlvinfo_header_t *hdr)
{

	int max_size = min(TLV_TOTAL_LEN_MAX, EEPROM_SIZE - sizeof(tlvinfo_header_t));

	return ((strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) && (hdr->version == TLV_INFO_VERSION)
			&& (be16_to_cpu(hdr->totallen) <= max_size));
}

/**
 *  is_hex
 *
 *  Tests if character is an ASCII hex digit
 */
static inline u8 is_hex(char p)
{
	return (((p >= '0') && (p <= '9')) || ((p >= 'A') && (p <= 'F')) || ((p >= 'a') && (p <= 'f')));
}

#endif
