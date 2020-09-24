#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include "eeprom_op.h"

enum tlv_position
{
	POS_VENDOR_NAME = 0,
	POS_PRODUCT_NAME,
	POS_PART_NUM,
	POS_SERIAL_NUM,
	POS_LABEL_REVISION,
	POS_MAC_1_BASE,
	POS_MANUFACTURE_DATE,
	POS_NUM_MACS,
	POS_MANUFACTURE,
	POS_COUNTRY_CODE,
	POS_VENDOR_EXTENSION,
	POS_CRC_32,
	POS_TLV_NUM,
};

struct tlvinfo_s
{
	u8 buf[EEPROM_SIZE];
};

enum show_method
{
	SHOW_METHOD_VERBOSE = 0, SHOW_METHOD_NAME_VALUE,
};

enum file_method
{
	CREATE_MODEL_FILE = 0,
	CREATE_MODEL_BY_PN = 1,
};
/**
 *  Struct for displaying the TLV codes and names.
 */
struct tlv_code_desc
{
	u8 m_code;
	char* m_name;
};

/**
 *  List of TLV codes and names.
 */
#if 1
static struct tlv_code_desc tlv_code_list[] =
{
{ TLV_CODE_PRODUCT_NAME, "product_name" },
{ TLV_CODE_PART_NUMBER, "part_number" },
{ TLV_CODE_SERIAL_NUMBER, "serial_number" },
{ TLV_CODE_MAC_BASE, "base_mac_address" },
{ TLV_CODE_MANUF_DATE, "manufacture_date" },
{ TLV_CODE_DEVICE_VERSION, "device_version" },
{ TLV_CODE_LABEL_REVISION, "label_revision" },
{ TLV_CODE_PLATFORM_NAME, "platform_name" },
{ TLV_CODE_ONIE_VERSION, "onie_version" },
{ TLV_CODE_MAC_SIZE, "mac_addresses" },
{ TLV_CODE_MANUF_NAME, "manufacturer" },
{ TLV_CODE_MANUF_COUNTRY, "country_code" },
{ TLV_CODE_VENDOR_NAME, "vendor_name" },
{ TLV_CODE_DIAG_VERSION, "diag_version" },
{ TLV_CODE_SERVICE_TAG, "service_tag" },
{ TLV_CODE_VENDOR_EXT, "vendor_extension" },
{ TLV_CODE_CRC_32, "crc_32" }, };
#else
static struct tlv_code_desc tlv_code_list[] =
{
	{	TLV_CODE_PRODUCT_NAME , "Product Name"},
	{	TLV_CODE_PART_NUMBER , "Part Number"},
	{	TLV_CODE_SERIAL_NUMBER , "Serial Number"},
	{	TLV_CODE_MAC_BASE , "Base MAC Address"},
	{	TLV_CODE_MANUF_DATE , "Manufacture Date"},
	{	TLV_CODE_DEVICE_VERSION, "Device Version"},
	{	TLV_CODE_LABEL_REVISION, "Label Revision"},
	{	TLV_CODE_PLATFORM_NAME , "Platform Name"},
	{	TLV_CODE_ONIE_VERSION , "ONIE Version"},
	{	TLV_CODE_MAC_SIZE , "MAC Addresses"},
	{	TLV_CODE_MANUF_NAME , "Manufacturer"},
	{	TLV_CODE_MANUF_COUNTRY , "Country Code"},
	{	TLV_CODE_VENDOR_NAME , "Vendor Name"},
	{	TLV_CODE_DIAG_VERSION , "Diag Version"},
	{	TLV_CODE_SERVICE_TAG , "Service Tag"},
	{	TLV_CODE_VENDOR_EXT , "Vendor Extension"},
	{	TLV_CODE_CRC_32 , "CRC-32"},
};
#endif

char * device_oem[POS_TLV_NUM] =
{ "Silicom", "ATT-V150", "80500-0150-G02", "012345678912", "RXXX", "mac", "2018-07-22", "128", "Silicom", "1122",
		"Silicom ONIE Version", };

int tlvinfo_tlv_value_size[POS_TLV_NUM] =
{
VENDOR_NAME_LEN,
PRODUCT_NAME_LEN,
PART_NUM_LEN,
SERIAL_NUM_LEN,
LABEL_REVISION_LEN,
MAC_1_BASE_LEN,
MANUFACTURE_DATE_LEN,
NUM_MACS_LEN,
MANUFACTURE_LEN,
COUNTRY_CODE_LEN,
VENDOR_EXTENSION_LEN,
CRC_32_LEN, };

char tlvinfo_tlv_code_type[POS_TLV_NUM] =
{
TLV_CODE_VENDOR_NAME,
TLV_CODE_PRODUCT_NAME,
TLV_CODE_PART_NUMBER,
TLV_CODE_SERIAL_NUMBER,
TLV_CODE_LABEL_REVISION,
TLV_CODE_MAC_BASE,
TLV_CODE_MANUF_DATE,
TLV_CODE_MAC_SIZE,
TLV_CODE_MANUF_NAME,
TLV_CODE_MANUF_COUNTRY,
TLV_CODE_VENDOR_EXT,
TLV_CODE_CRC_32, };

static struct tlvinfo_s eeprom_inst;
static char file_path[] = "/sys/bus/i2c/devices/0-0050/eeprom";
static char file_path_bus0[] = "/sys/bus/i2c/devices/0-0050/eeprom";
static char file_path_bus1[] = "/sys/bus/i2c/devices/1-0050/eeprom";
//static char file_path[] = "eeprom_file";

extern uint32_t crc32(uint32_t crc, const char *p, uint32_t len);

static bool tlvinfo_find_tlv(u8 * eeprom, u8 tcode, int * eeprom_index);
static void update_crc(u8 *eeprom);

/**
 *  Look up a TLV name by its type.
 */
static inline const char* tlv_type2name(u8 type)
{
	char* name = "Unknown";
	int i;

	for (i = 0; i < sizeof(tlv_code_list) / sizeof(tlv_code_list[0]); i++)
	{
		if (tlv_code_list[i].m_code == type)
		{
			name = tlv_code_list[i].m_name;
			break;
		}
	}

	return name;
}

static inline const u8 tlv_name2type(char* name)
{
	u8 tag = 0;
	int i;

	for (i = 0; i < sizeof(tlv_code_list) / sizeof(tlv_code_list[0]); i++)
	{
		if (strncmp(name, tlv_code_list[i].m_name, strlen(tlv_code_list[i].m_name)) == 0)
		{
			tag = tlv_code_list[i].m_code;
			break;
		}
	}

	return tag;
}

/*
 *  decode_tlv
 *
 *  Print a string representing the contents of the TLV field. The format of
 *  the string is:
 *      1. The name of the field left justified in 20 characters
 *      2. The type code in hex right justified in 5 characters
 *      3. The length in decimal right justified in 4 characters
 *      4. The value, left justified in however many characters it takes
 *  The validity of EEPROM contents and the TLV field have been verified
 *  prior to calling this function.
 */
#define DECODE_NAME_MAX     20

/*
 * The max decode value is currently for the 'raw' type or the 'vendor
 * extension' type, both of which have the same decode format.  The
 * max decode string size is computed as follows:
 *
 *   strlen(" 0xFF") * TLV_VALUE_MAX_LEN + 1
 *
 */
#define DECODE_VALUE_MAX    ((5 * TLV_VALUE_MAX_LEN) +  1)

static void decode_tlv(tlvinfo_tlv_t * tlv, enum show_method method)
{
	char name[DECODE_NAME_MAX];
	char value[DECODE_VALUE_MAX];
	int i;

	strncpy(name, tlv_type2name(tlv->type), DECODE_NAME_MAX);
	name[DECODE_NAME_MAX - 1] = 0;

	switch (tlv->type)
	{
	case TLV_CODE_PRODUCT_NAME:
	case TLV_CODE_PART_NUMBER:
	case TLV_CODE_SERIAL_NUMBER:
	case TLV_CODE_MANUF_DATE:
	case TLV_CODE_LABEL_REVISION:
	case TLV_CODE_PLATFORM_NAME:
	case TLV_CODE_ONIE_VERSION:
	case TLV_CODE_MANUF_NAME:
	case TLV_CODE_MANUF_COUNTRY:
	case TLV_CODE_VENDOR_NAME:
	case TLV_CODE_DIAG_VERSION:
	case TLV_CODE_SERVICE_TAG:
		memcpy(value, tlv->value, tlv->length);
		value[tlv->length] = 0;
		break;
	case TLV_CODE_MAC_BASE:
		sprintf(value, "%02X:%02X:%02X:%02X:%02X:%02X", tlv->value[0], tlv->value[1], tlv->value[2], tlv->value[3],
				tlv->value[4], tlv->value[5]);
		break;
	case TLV_CODE_DEVICE_VERSION:
		sprintf(value, "%u", tlv->value[0]);
		break;
	case TLV_CODE_MAC_SIZE:
		sprintf(value, "%u", (tlv->value[0] << 8) | tlv->value[1]);
		break;
	case TLV_CODE_VENDOR_EXT:
		// display the vendor ext info after IANA code
		memcpy(value, tlv->value+4, tlv->length-4);
		value[tlv->length-4] = 0;
		printf("%s(ASCII)=%s\n", name, value);
		value[0] = 0;
		for (i = 0; (i < (DECODE_VALUE_MAX / 5)) && (i < tlv->length); i++)
		{
			if (i == 0)
				sprintf(value, "0x%02X", tlv->value[i]);
			else
				sprintf(value, "%s,0x%02X", value, tlv->value[i]);
		}
		break;
	case TLV_CODE_CRC_32:
		sprintf(value, "0x%02X%02X%02X%02X", tlv->value[0], tlv->value[1], tlv->value[2], tlv->value[3]);
		break;
	default:
		value[0] = 0;
		for (i = 0; (i < (DECODE_VALUE_MAX / 5)) && (i < tlv->length); i++)
		{
			sprintf(value, "%s 0x%02X", value, tlv->value[i]);
		}
		break;
	}

	if (method == SHOW_METHOD_NAME_VALUE)
	{
		if (tlv->type != TLV_CODE_CRC_32)
			printf("%s=%s\n", name, value);
	}
	else
		printf("%-20s 0x%02X %3d %s\n", name, tlv->type, tlv->length, value);

	return;
}

/**
 *  is_valid_tlv
 *
 *  Perform basic sanity checks on a TLV field. The TLV is pointed to
 *  by the parameter provided.
 *      1. The type code is not reserved (0x00 or 0xFF)
 */
static inline bool is_valid_tlv(tlvinfo_tlv_t *tlv)
{
	return ((tlv->type != 0x00) && (tlv->type != 0xFF));
}

/**
 *  is_checksum_valid
 *
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 */
static bool is_checksum_valid(u8 *eeprom)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_crc;
	unsigned int calc_crc;
	unsigned int stored_crc;

	// Is the eeprom header valid?
	if (!is_valid_tlvinfo_header(eeprom_hdr))
	{
		return (FALSE);
	}

	// Is the last TLV a CRC?
	eeprom_crc = (tlvinfo_tlv_t *) &eeprom[sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen)
			- (sizeof(tlvinfo_tlv_t) + 4)];
	if ((eeprom_crc->type != TLV_CODE_CRC_32) || (eeprom_crc->length != 4))
	{
		return (FALSE);
	}

	// Calculate the checksum
	calc_crc = crc32(0, (void *) eeprom, sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) - 4);
	stored_crc = (eeprom_crc->value[0] << 24) | (eeprom_crc->value[1] << 16) | (eeprom_crc->value[2] << 8)
			| eeprom_crc->value[3];
	return (calc_crc == stored_crc);
}

/**
 *  tlvinfo_delete_tlv
 *
 *  This function deletes the TLV with the specified type code from the
 *  EEPROM.
 */
static bool tlvinfo_delete_tlv(u8 * eeprom, u8 code)
{
	int eeprom_index;
	int tlength;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;

	// Find the TLV and then move all following TLVs "forward"
	if (tlvinfo_find_tlv(eeprom, code, &eeprom_index))
	{
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
		tlength = sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
		memcpy(&eeprom[eeprom_index], &eeprom[eeprom_index + tlength],
				sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) - eeprom_index - tlength);
		eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) - tlength);
		update_crc(eeprom);
		return (TRUE);
	}
	return (FALSE);
}

/**
 *  set_mac
 *
 *  Converts a string MAC address into a binary buffer.
 *
 *  This function takes a pointer to a MAC address string
 *  (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number).
 *  The string format is verified and then converted to binary and
 *  stored in a buffer.
 */
static int set_mac(char *buf, const char *string)
{
	char *p = (char *) string;
	int i;
	int err = 0;
	char *end;

	if (!p)
	{
		printf("ERROR: NULL mac addr string passed in.\n");
		return -1;
	}

	if (strlen(p) != 17)
	{
		printf("ERROR: MAC address strlen() != 17 -- %d\n", strlen(p));
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	for (i = 0; i < 17; i++)
	{
		if ((i % 3) == 2)
		{
			if (p[i] != ':')
			{
				err;
				printf("ERROR: mac: p[%i] != :, found: `%c'\n", i, p[i]);
				break;
			}
			continue;
		}
		else if (!is_hex(p[i]))
		{
			err;
			printf("ERROR: mac: p[%i] != hex digit, found: `%c'\n", i, p[i]);
			break;
		}
	}

	if (err != 0)
	{
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	/* Convert string to binary */
	for (i = 0, p = (char *) string; i < 6; i++)
	{
		buf[i] = p ? strtoul(p, &end, 16) : 0;
		if (p)
		{
			p = (*end) ? end + 1 : end;
		}
	}

#if 0
	if (!is_valid_ether_addr((u8 *)buf))
	{
		printf("ERROR: MAC address must not be 00:00:00:00:00:00, "
				"a multicast address or FF:FF:FF:FF:FF:FF.\n");
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}
#endif

	return 0;
}

/**
 *  set_date
 *
 *  Validates the format of the data string
 *
 *  This function takes a pointer to a date string (i.e. MM/DD/YYYY hh:mm:ss)
 *  and validates that the format is correct. If so the string is copied
 *  to the supplied buffer.
 */
static int set_date(char *buf, const char *string)
{
	int i;

	if (!string)
	{
		printf("ERROR: NULL date string passed in.\n");
		return -1;
	}

	if (strlen(string) != 19)
	{
		printf("ERROR: Date strlen() != 19 -- %d\n", strlen(string));
		printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
		return -1;
	}

	for (i = 0; string[i] != 0; i++)
	{
		switch (i)
		{
		case 2:
		case 5:
			if (string[i] != '/')
			{
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		case 10:
			if (string[i] != ' ')
			{
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		case 13:
		case 16:
			if (string[i] != ':')
			{
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		default:
			if (!is_digit(string[i]))
			{
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		}
	}

	strcpy(buf, string);
	return 0;
}

/**
 *  set_bytes
 *
 *  Converts a space-separated string of decimal numbers into a
 *  buffer of bytes.
 *
 *  This function takes a pointer to a space-separated string of decimal
 *  numbers (i.e. "128 0x55 0321") with "C" standard radix specifiers
 *  and converts them to an array of bytes.
 */
static int set_bytes(char *buf, const char *string, int * converted_accum)
{
	char *p = (char *) string;
	int i = 0;
	uint byte;

	if (!p)
	{
		printf("ERROR: NULL string passed in.\n");
		return -1;
	}

	/* Convert string to bytes */
	while(*p && i < TLV_VALUE_MAX_LEN)
	{
		byte = strtoul(p, NULL, 0);
		if (byte >= 256)
		{
			printf("ERROR: The value specified is greater than 255: (%u) in string: %s\n", byte, string);
			return -1;
		}
		buf[i] = byte & 0xFF;
		i++;
		p = strstr(p, ",");
		if(p)
			p++;
		else
			break;
	}

	if ((i == TLV_VALUE_MAX_LEN) && (*p != 0))
	{
		printf("ERROR: Trying to assign too many bytes "
				"(max: %d) in string: %s\n", TLV_VALUE_MAX_LEN, string);
		return -1;
	}

	*converted_accum = i;
	return 0;
}

/**
 *  tlvinfo_add_tlv
 *
 *  This function adds a TLV to the EEPROM, converting the value (a string) to
 *  the format in which it will be stored in the EEPROM.
 */
#define MAX_TLV_VALUE_LEN   256
static bool tlvinfo_add_tlv(u8 * eeprom, int tcode, char * strval)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;
	int new_tlv_len = 0;
	u32 value;
	char data[MAX_TLV_VALUE_LEN];
	int eeprom_index;
	int max_size = min(TLV_TOTAL_LEN_MAX, EEPROM_SIZE - sizeof(tlvinfo_header_t));

	// Encode each TLV type into the format to be stored in the EERPOM
	switch (tcode)
	{
	case TLV_CODE_PRODUCT_NAME:
	case TLV_CODE_PART_NUMBER:
	case TLV_CODE_SERIAL_NUMBER:
	case TLV_CODE_LABEL_REVISION:
	case TLV_CODE_PLATFORM_NAME:
	case TLV_CODE_ONIE_VERSION:
	case TLV_CODE_MANUF_NAME:
	case TLV_CODE_MANUF_COUNTRY:
	case TLV_CODE_VENDOR_NAME:
	case TLV_CODE_DIAG_VERSION:
	case TLV_CODE_SERVICE_TAG:
		strncpy(data, strval, MAX_TLV_VALUE_LEN);
		new_tlv_len = min(MAX_TLV_VALUE_LEN, strlen(strval));
		break;
	case TLV_CODE_DEVICE_VERSION:
		value = strtoul(strval, NULL, 0);
		if (value >= 256)
		{
			printf("ERROR: Device version must be 255 or less. Value supplied: %u", value);
			return (FALSE);
		}
		data[0] = value & 0xFF;
		new_tlv_len = 1;
		break;
	case TLV_CODE_MAC_SIZE:
		value = strtoul(strval, NULL, 0);
		if (value >= 65536)
		{
			printf("ERROR: MAC Size must be 65535 or less. Value supplied: %u", value);
			return (FALSE);
		}
		data[0] = (value >> 8) & 0xFF;
		data[1] = value & 0xFF;
		new_tlv_len = 2;
		break;
	case TLV_CODE_MANUF_DATE:
		if (set_date(data, strval) != 0)
		{
			return (FALSE);
		}
		new_tlv_len = 19;
		break;
	case TLV_CODE_MAC_BASE:
		if (set_mac(data, strval) != 0)
		{
			return (FALSE);
		}
		new_tlv_len = 6;
		break;
	case TLV_CODE_CRC_32:
		printf("WARNING: The CRC TLV is set automatically and cannot be set manually.\n");
		return (FALSE);
	case TLV_CODE_VENDOR_EXT:
	default:
		if (set_bytes(data, strval, &new_tlv_len) != 0)
		{
			return (FALSE);
		}
		break;
	}

	// Is there room for this TLV?
	if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + new_tlv_len) > max_size)
	{
		printf("ERROR: There is not enough room in the EERPOM to save data.\n");
		return (FALSE);
	}

	// Add TLV at the end, overwriting CRC TLV if it exists
	if (tlvinfo_find_tlv(eeprom, TLV_CODE_CRC_32, &eeprom_index))
	{
		eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) - sizeof(tlvinfo_tlv_t) - 4);
	}
	else
	{
		eeprom_index = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	}
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	eeprom_tlv->type = tcode;
	eeprom_tlv->length = new_tlv_len;
	memcpy(eeprom_tlv->value, data, new_tlv_len);

	// Update the total length and calculate (add) a new CRC-32 TLV
	eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + new_tlv_len);
	update_crc(eeprom);

	return (TRUE);
}

static void eeprom_dump(u8 *eeprom)
{
	int i = 0;
	printf("EEPROM dump: (0x%x bytes)", TLV_INFO_MAX_LEN);
	for (i = 0; i < TLV_INFO_MAX_LEN; i++)
	{
		if ((i % 16) == 0)
			printf("\n%02X: ", i++);
		printf("%02X ", eeprom[i]);
	}
	printf("\n");

}
/*
 *Just to create a file in /etc/product/UBMC/OEMI/model.txt
 *This file contain the machine model information.
 *
 * */
#define OEMI_FILE_ROOT_PATH "/etc/product/UBMC"

#define OEMI_FILE_PATH "/etc/product/UBMC/OEMI"
#define OEMI_FILE_NAME "OEMI"

#define OEMI_MODEL_FILE_PATH "/etc/product/UBMC/OEMI/model.txt"
#define OEMI_MODEL_FILE_NAME "model.txt"

static int create_model_file(u8 *eeprom)
{
	int tlv_end;
	int curr_tlv;
	int ret;
	int fd;
	char name[DECODE_NAME_MAX];
	char value[DECODE_VALUE_MAX];
	int  eeprom_index;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;
	if (!is_valid_tlvinfo_header(eeprom_hdr))
	{
		printf("EEPROM does not contain data in a valid TlvInfo format.\n");
		return 1;
	}
	if(access(OEMI_FILE_ROOT_PATH,F_OK) != 0)
	{
		printf("can not find  %s fail :%s",OEMI_FILE_ROOT_PATH,strerror(errno));
		return 1;
	}
	if(access(OEMI_FILE_PATH,F_OK) != 0)
	{
		ret = mkdir(OEMI_FILE_PATH,S_IRWXU|S_IROTH|S_IRGRP|S_IXOTH|S_IXGRP);
		if(ret < 0)
		{
			printf("create %s fail :%s",OEMI_FILE_PATH,strerror(errno));
			return 1;
		}
	}
	fd = open(OEMI_MODEL_FILE_PATH,O_CREAT|O_WRONLY,S_IRWXU|S_IROTH|S_IRGRP);
	if(fd < 0)
	{
		printf("create %s fail :%s",OEMI_MODEL_FILE_PATH,strerror(errno));
		return 1;
	}
	ret = tlvinfo_find_tlv(eeprom, TLV_CODE_PRODUCT_NAME, &eeprom_index);
	if(ret < 0)
	{
		printf("tlvinfo_find_tlv fail \n");
	}
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	strncpy(name, tlv_type2name(eeprom_tlv->type), DECODE_NAME_MAX);
	name[DECODE_NAME_MAX - 1] = 0;
	memcpy(value, eeprom_tlv->value, eeprom_tlv->length);
	value[eeprom_tlv->length] = 0;
//	printf("%s:%s \n",name,value);
	ret = write(fd,value,eeprom_tlv->length);
	if(ret < 0)
	{
		printf("write product name %s to OEMI Model file fail:%s",value,strerror(errno));
		close(fd);
		return 1;
	}
	close(fd);
	return 0;

}

#define UBMC_XS_PN	"80500-0150"
#define UBMC_S_PN	"80500-0179"
#define UBMC_M_PN	"80500-0180"
#define UBMC_L_PN	"80500-0181"
#define UBMC_SKYD_PN	"80400-0152"
#define UBMC_SKYD_EVT_PN	"90500-0168"

#define UBMC_XS_MODEL	"ATT-V150"
#define UBMC_S_MODEL	"ATT-V250"
#define UBMC_M_MODEL	"ATT-V450"
#define UBMC_L_MODEL	"ATT-V850"
#define UBMC_SKYD_MODEL	"Barcelona"
#define UBMC_SKYD_EVT_MODEL	"DU-SKYD-EVT"
#define UBMC_PN_MAXSIZE 10
static int create_model_file_by_pn(u8 *eeprom)
{
	int tlv_end;
	int curr_tlv;
	int ret;
	int fd;
	char *pos;
	char slen;
	char part_num[DECODE_NAME_MAX];
	char mode_type[DECODE_NAME_MAX]={0};
	char model[DECODE_VALUE_MAX];
	int  eeprom_index;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;
/*	if (!is_valid_tlvinfo_header(eeprom_hdr))
	{
		printf("EEPROM does not contain data in a valid TlvInfo format.\n");
		eeprom_err("EEPROM does not contain data in a valid TlvInfo format.\n");
		return 1;
	}
	*/
	if(access(OEMI_FILE_ROOT_PATH,F_OK) != 0)
	{
		printf("can not find  %s fail :%s",OEMI_FILE_ROOT_PATH,strerror(errno));
		eeprom_err("can not find  %s fail :%s",OEMI_FILE_ROOT_PATH,strerror(errno));
		return 1;
	}
	if(access(OEMI_FILE_PATH,F_OK) != 0)
	{
		ret = mkdir(OEMI_FILE_PATH,S_IRWXU|S_IROTH|S_IRGRP|S_IXOTH|S_IXGRP);
		if(ret < 0)
		{
			printf("create %s fail :%s",OEMI_FILE_PATH,strerror(errno));
			eeprom_err("create %s fail :%s",OEMI_FILE_PATH,strerror(errno));
			return 1;
		}
	}
	fd = open(OEMI_MODEL_FILE_PATH,O_CREAT|O_WRONLY,S_IRWXU|S_IROTH|S_IRGRP);
	if(fd < 0)
	{
		printf("create %s fail :%s",OEMI_MODEL_FILE_PATH,strerror(errno));
		eeprom_err("create %s fail :%s",OEMI_MODEL_FILE_PATH,strerror(errno));
		return 1;
	}
	ret = tlvinfo_find_tlv(eeprom, TLV_CODE_PART_NUMBER, &eeprom_index);
	if(ret < 0)
	{
		printf("tlvinfo_find_tlv fail \n");
	}
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	strncpy(part_num, eeprom_tlv->value, DECODE_NAME_MAX);
	part_num[DECODE_NAME_MAX - 1] = 0;
	//printf("part_num is %s \n",part_num);
	//printf("mode_type is %s \n",mode_type);
	//printf("value is %s \n",eeprom_tlv->value);
	if(strncmp(part_num,UBMC_XS_PN,UBMC_PN_MAXSIZE) == 0)
	{
		strcpy(model, UBMC_XS_MODEL);
	}
	else if(strncmp(part_num,UBMC_S_PN,UBMC_PN_MAXSIZE) == 0)
	{
		strcpy(model, UBMC_S_MODEL);
	}
	else if(strncmp(part_num,UBMC_M_PN,UBMC_PN_MAXSIZE) == 0)
	{
		strcpy(model, UBMC_M_MODEL);
	}
	else if(strncmp(part_num,UBMC_L_PN,UBMC_PN_MAXSIZE) == 0)
	{
		strcpy(model, UBMC_L_MODEL);
	}
	else if(strncmp(part_num,UBMC_SKYD_PN,UBMC_PN_MAXSIZE) == 0)
	{
		strcpy(model, UBMC_SKYD_MODEL);
	}
	else if(strncmp(part_num,UBMC_SKYD_EVT_PN,UBMC_PN_MAXSIZE) == 0)
	{
		strcpy(model, UBMC_SKYD_EVT_MODEL);
	}
	else
	{
		strcpy(model, "NONE");
	}
	slen = strlen(model);
	model[slen] = '\0';
//	printf("%s:%s \n",name,value);
//	printf("mode is %s \n",model);
	ret = write(fd,model,slen);
	if(ret < 0)
	{
		printf("write product name %s to OEMI Model file fail:%s",model,strerror(errno));
		eeprom_err("write product name %s to OEMI Model file fail:%s",model,strerror(errno));
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

static void show_eeprom_verboose(u8 *eeprom)
{
	int tlv_end;
	int curr_tlv;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;

	if (!is_valid_tlvinfo_header(eeprom_hdr))
	{
		printf("EEPROM does not contain data in a valid TlvInfo format.\n");
		return;
	}

	printf("TlvInfo Header:\n");
	printf("   Id String:    %s\n", eeprom_hdr->signature);
	printf("   Version:      %d\n", eeprom_hdr->version);
	printf("   Total Length: %d\n", be16_to_cpu(eeprom_hdr->totallen));

	printf("TLV Name             Code Len Value\n");
	printf("-------------------- ---- --- -----\n");
	curr_tlv = sizeof(tlvinfo_header_t);
	tlv_end = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	printf("header size %d\n", curr_tlv);
	while (curr_tlv < tlv_end)
	{
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[curr_tlv];
		if (!is_valid_tlv(eeprom_tlv))
		{
			printf("Invalid TLV field starting at EEPROM offset %d\n", curr_tlv);
			return;
		}
		decode_tlv(eeprom_tlv, SHOW_METHOD_VERBOSE);
		curr_tlv += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
	}

	printf("Checksum is %s.\n", is_checksum_valid(eeprom) ? "valid" : "invalid");

#ifdef DEBUG
	eeprom_dump(eeprom);
#endif

	return;
}

static void show_eeprom_name_value(u8 *eeprom)
{
	int tlv_end;
	int curr_tlv;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;

	if (!is_valid_tlvinfo_header(eeprom_hdr))
	{
		printf("Error: EEPROM does not contain data in a valid TlvInfo format.\n");
		return;
	}

	if (!is_checksum_valid(eeprom))
	{
		printf("Error: EEPROM CRC_32 Checksum is invalid.\n");
		return;
	}

	curr_tlv = sizeof(tlvinfo_header_t);
	tlv_end = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	while (curr_tlv < tlv_end)
	{
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[curr_tlv];
		if (!is_valid_tlv(eeprom_tlv))
		{
			printf("Invalid TLV field starting at EEPROM offset %d\n", curr_tlv);
			return;
		}
		decode_tlv(eeprom_tlv, SHOW_METHOD_NAME_VALUE);
		curr_tlv += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
	}

#ifdef DEBUG
	eeprom_dump(eeprom);
#endif
	return;
}

/**
 *  tlvinfo_find_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM.
 *  An offset from the beginning of the EEPROM is returned in the
 *  eeprom_index parameter if the TLV is found.
 */
static bool tlvinfo_find_tlv(u8 * eeprom, u8 tcode, int * eeprom_index)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;
	int eeprom_end;
	int i = 0;

	// Search through the TLVs, looking for the first one which matches the
	// supplied type code.
	*eeprom_index = sizeof(tlvinfo_header_t);
	eeprom_end = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);

	while (*eeprom_index < eeprom_end)
	{
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[*eeprom_index];
		if (!is_valid_tlv(eeprom_tlv))
		{
			return (FALSE);
		}
		if (eeprom_tlv->type == tcode)
		{
			return (TRUE);
		}
		*eeprom_index += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
	}

	return (FALSE);
}

/**
 *  update_crc
 *
 *  This function updates the CRC-32 TLV. If there is no CRC-32 TLV, then
 *  one is added. This function should be called after each update to the
 *  EEPROM structure, to make sure the CRC is always correct.
 */
static void update_crc(u8 *eeprom)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_crc;
	unsigned int calc_crc;
	int eeprom_index;

	printf("Find CRC\n");
	// Discover the CRC TLV
	if (!tlvinfo_find_tlv(eeprom, TLV_CODE_CRC_32, &eeprom_index))
	{
		printf("CRC not found, add it\n");
		if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + 4) > TLV_TOTAL_LEN_MAX)
		{
			return;
		}
		eeprom_index = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
		eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + 4);
	}
	printf("Write CRC idx=%d total=%d\n", eeprom_index, be16_to_cpu(eeprom_hdr->totallen));
	eeprom_crc = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	eeprom_crc->type = TLV_CODE_CRC_32;
	eeprom_crc->length = 4;

	// Calculate the checksum
	calc_crc = crc32(0, (void *) eeprom, sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) - 4);
	eeprom_crc->value[0] = (calc_crc >> 24) & 0xFF;
	eeprom_crc->value[1] = (calc_crc >> 16) & 0xFF;
	eeprom_crc->value[2] = (calc_crc >> 8) & 0xFF;
	eeprom_crc->value[3] = (calc_crc >> 0) & 0xFF;

	//printf("0x%x %x %x %x\n",eeprom_crc->value[0],eeprom_crc->value[1],eeprom_crc->value[2],eeprom_crc->value[3]);
	return;
}

int eeprom_op_tlvinfo_init(struct tlvinfo_s * p_eeprom_inst)
{
	memset(p_eeprom_inst->buf, 0, EEPROM_SIZE);
	return 0;
}
int eeprom_op_tlvinfo_tlv_size_sum()
{
	int i = 0;
	int sum = 0;

	for (i = 0; i < POS_TLV_NUM; i++)
	{
		sum += tlvinfo_tlv_value_size[i];
	}

	return sum;
}

int eeprom_op_tlvinfo_default_init(u8 *eeprom)
{
	tlvinfo_header_t * eeprom_hdr;
	tlvinfo_tlv_t * eeprom_tlv;
	int eeprom_index, eeprom_end;
	unsigned short total_len;

	int i = 0;

	eeprom_hdr = (tlvinfo_header_t *) eeprom;

	strncpy(eeprom_hdr->signature, TLV_INFO_ID_STRING, ID_STRING_LEN);
	eeprom_hdr->version = TLV_INFO_VERSION;

	total_len = (POS_TLV_NUM * 2 + eeprom_op_tlvinfo_tlv_size_sum());
	eeprom_hdr->totallen = cpu_to_be16(total_len);

	eeprom_index = sizeof(tlvinfo_header_t);
	eeprom_end = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);

	/*
	 *CRC_32 need to be init at update_crc,so i  < TLV_NUM - 1
	 */
	while (eeprom_index < eeprom_end && i < POS_TLV_NUM - 1)
	{
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
		eeprom_tlv->type = tlvinfo_tlv_code_type[i];
		eeprom_tlv->length = tlvinfo_tlv_value_size[i];
		memcpy(eeprom_tlv->value, device_oem[i],
				eeprom_tlv->length < strlen(device_oem[i]) ? eeprom_tlv->length : strlen(device_oem[i]));

		eeprom_index += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
		i++;
	}
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	eeprom_tlv->type = tlvinfo_tlv_code_type[POS_CRC_32];
	eeprom_tlv->length = tlvinfo_tlv_value_size[POS_CRC_32];

	update_crc(eeprom);
	return 0;
}

static int prog_eeprom(int fd, u8 *eeprom)
{
	int ret = 0;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	int eeprom_len;

	update_crc(eeprom);

	eeprom_len = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	ret = write(fd, eeprom, eeprom_len);
	if (!ret)
	{
		printf("Programming failed.\n");
		return -1;
	}

	printf("Programming passed.\n");
	return 0;
}

static int write_eeprom(u8 *eeprom)
{
	int fd;

	fd = open(file_path, O_CREAT | O_RDWR);
	if (fd < 0)
	{
		printf("Fail to access eeprom \n");
		return -1;
	}

//	eeprom_op_tlvinfo_default_init(eeprom_inst.buf);
	prog_eeprom(fd, eeprom);

	close(fd);

	return 0;
}

static int read_eeprom(u8 *eeprom, int eeprom_len)
{
	int fd;

	fd = open(file_path, O_CREAT | O_RDWR);
	if (fd < 0)
	{
		printf("Fail to access eeprom \n");
		return -1;
	}
	read(fd, eeprom, eeprom_len);
	close(fd);

	return 0;
}

static int create_eeprom_info_file(enum file_method method)
{
	u8 *eeprom = eeprom_inst.buf;
	int ret = 0;
	if (read_eeprom(eeprom, EEPROM_SIZE))
		return -1;
	switch (method)
	{
	case CREATE_MODEL_FILE:
		ret = create_model_file(eeprom);
		break;
	case CREATE_MODEL_BY_PN:
		ret = create_model_file_by_pn(eeprom);
		break;
	default:
		break;
	}
	return ret;
}

static int check_eeprom_data(void)
{
	u8 *eeprom = eeprom_inst.buf;
	int ret = 0;
	if (read_eeprom(eeprom, EEPROM_SIZE))
		return -1;
	ret = is_checksum_valid(eeprom);
	if(!ret)
	{
		return -1;
	}
	return 0;
}

/**
 *  show_eeprom
 *
 *  Display the contents of the EEPROM
 */
static void show_eeprom(enum show_method method)
{
	u8 *eeprom = eeprom_inst.buf;

	if (read_eeprom(eeprom, EEPROM_SIZE))
		return;

	switch (method)
	{
	case SHOW_METHOD_VERBOSE:
		show_eeprom_verboose(eeprom);
		break;
	case SHOW_METHOD_NAME_VALUE:
		show_eeprom_name_value(eeprom);
		break;
	default:
		break;
	}
}

static u8* add_eeprom(u8* eeprom, int tcode, char * strval)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;
	int new_tlv_len = 0;
	u32 value;
	char data[MAX_TLV_VALUE_LEN];
	int eeprom_index;
	int max_size = min(TLV_TOTAL_LEN_MAX, EEPROM_SIZE - sizeof(tlvinfo_header_t));

	// Encode each TLV type into the format to be stored in the EERPOM
	switch (tcode)
	{
	case TLV_CODE_PRODUCT_NAME:
	case TLV_CODE_PART_NUMBER:
	case TLV_CODE_SERIAL_NUMBER:
	case TLV_CODE_LABEL_REVISION:
	case TLV_CODE_PLATFORM_NAME:
	case TLV_CODE_ONIE_VERSION:
	case TLV_CODE_MANUF_NAME:
	case TLV_CODE_MANUF_COUNTRY:
	case TLV_CODE_VENDOR_NAME:
	case TLV_CODE_DIAG_VERSION:
	case TLV_CODE_SERVICE_TAG:
		strncpy(data, strval, MAX_TLV_VALUE_LEN);
		new_tlv_len = min(MAX_TLV_VALUE_LEN, strlen(strval));
		break;
	case TLV_CODE_DEVICE_VERSION:
		value = strtoul(strval, NULL, 0);
		if (value >= 256)
		{
			printf("ERROR: Device version must be 255 or less. Value supplied: %u", value);
			return (FALSE);
		}
		data[0] = value & 0xFF;
		new_tlv_len = 1;
		break;
	case TLV_CODE_MAC_SIZE:
		value = strtoul(strval, NULL, 0);
		if (value >= 65536)
		{
			printf("ERROR: MAC Size must be 65535 or less. Value supplied: %u", value);
			return (FALSE);
		}
		data[0] = (value >> 8) & 0xFF;
		data[1] = value & 0xFF;
		new_tlv_len = 2;
		break;
	case TLV_CODE_MANUF_DATE:
		if (set_date(data, strval) != 0)
		{
			return (FALSE);
		}
		new_tlv_len = 19;
		break;
	case TLV_CODE_MAC_BASE:
		if (set_mac(data, strval) != 0)
		{
			return (FALSE);
		}
		new_tlv_len = 6;
		break;
	case TLV_CODE_CRC_32:
		printf("WARNING: The CRC TLV is set automatically and cannot be set manually.\n");
		return (FALSE);
	case TLV_CODE_VENDOR_EXT:
	default:
		if (set_bytes(data, strval, &new_tlv_len) != 0)
		{
			return (FALSE);
		}
		break;
	}
	printf("Add TLV tag=%d len=%d\n", tcode, new_tlv_len);

	// Is there room for this TLV?
	if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + new_tlv_len) > max_size)
	{
		printf("ERROR: There is not enough room in the EERPOM to save data.\n");
		return (FALSE);
	}

	eeprom_index = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	printf("eeprom_index=%d\n", eeprom_index);

	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	eeprom_tlv->type = tcode;
	eeprom_tlv->length = new_tlv_len;
	memcpy(eeprom_tlv->value, data, new_tlv_len);

	// Update the total length and calculate (add) a new CRC-32 TLV
	eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + new_tlv_len);

	printf("Add eeprom OK\n");
	return (TRUE);
}

static int update_eeprom(char* arg)
{
	char* name;
	char* value;
	char value_str[EEPROM_SIZE*4];
	u8 eeprom_src[EEPROM_SIZE], eeprom_dst[EEPROM_SIZE];
	u8 tag = 0;
	int target_idx = 0, curr_idx = 0;
	tlvinfo_header_t *hdr_src, *hdr_dst;
	tlvinfo_tlv_t *tlv_src, *tlv_dst;
	int r_len = 0, r_totallen = 0, w_len = 0;
	int need_add = 1;

	if(!arg)
	{
		printf("Null argument\n");
		return -1;
	}
	tag = tlv_name2type(arg);
	if(tag == 0)
	{
		printf("fail to find tag, arg=%s\n", arg);
		return -1;
	}
	name = tlv_type2name(tag);
	if(arg[strlen(name)] == '.')
	{
		printf("arg with idx\n");
		target_idx = atoi(&arg[strlen(name)+1]);
	}
	value = strstr(arg, "=");
	if(!value)
	{
		printf("fail to find value arg=%s\n", arg);
		return -1;
	}
	value++;
	if(tag == TLV_CODE_VENDOR_EXT && value[0])
	{
		int i, len = strlen(value);
		char sec[8];
		strcpy(value_str, "0x00,0x00,0x3D,0x4E");
		for(i=0; i<len; i++)
		{
			sprintf(sec, ",0x%02X", value[i]);
			strcat(value_str, sec);
		}
		value = value_str;
	}
	printf("name=%s, tag=%d, idx=%d, value=%s\n", name, tag, target_idx, value);

	if (read_eeprom(eeprom_src, EEPROM_SIZE))
	{
		printf("fail to read eeprom\n");
		return -1;
	}
	hdr_src = (tlvinfo_header_t *) eeprom_src;
	hdr_dst = (tlvinfo_header_t *) eeprom_dst;
	memcpy(hdr_dst, hdr_src, sizeof(tlvinfo_header_t));
	hdr_dst->totallen = cpu_to_be16(0);

	r_len = w_len = sizeof(tlvinfo_header_t);
	r_totallen = sizeof(tlvinfo_header_t) + be16_to_cpu(hdr_src->totallen);
	while(r_len < r_totallen)
	{
		tlv_src = (tlvinfo_tlv_t *) &eeprom_src[r_len];
		tlv_dst = (tlvinfo_tlv_t *) &eeprom_dst[w_len];
		if(tlv_src->type == TLV_CODE_CRC_32)
			break;
		if(tlv_src->type == tag && curr_idx == target_idx)
		{
			if(!value[0])
			{
				printf("TLV value is null, delete it\n");
				need_add = 0;
			}
			else
			{
				printf("TLV found, update it\n");
				if(!add_eeprom(eeprom_dst, tag, value))
					return -1;
				need_add = 0;
			}
		}
		else
		{
			memcpy(tlv_dst, tlv_src, sizeof(tlvinfo_tlv_t)+tlv_src->length);
			hdr_dst->totallen = cpu_to_be16(be16_to_cpu(hdr_dst->totallen) + sizeof(tlvinfo_tlv_t) + tlv_src->length);
			if(tlv_src->type == tag)
				curr_idx++;
		}
		r_len += sizeof(tlvinfo_tlv_t)+tlv_src->length;
		w_len = sizeof(tlvinfo_header_t) + be16_to_cpu(hdr_dst->totallen);
	}
	if(need_add)
	{
		printf("TLV not found, add it\n");
		if(!add_eeprom(eeprom_dst, tag, value))
			return -1;
	}

	return write_eeprom(eeprom_dst);
}

void usage(char * prog)
{
	fprintf(stderr,
			"usage: %s [-h] [-v] [-r] [-w <name>[.<idx>]=<value>]\n \
            -h : help \n \
            -v : read in verbose \n \
            -r : raw read \n \
            -w : raw wirte default \n",
			prog);
}

#define PRODUCT_SUB_PATH "/etc/product_sub.txt"
#define UBMC_SUB_NAME_MAX 20
#define UBMC_M_NAME "UBMC_M"

#define UBMC_DEFAULT 0
#define UBMC_M 1
int get_machine_prod_sub(void)
{
	int ret;
	int ubmc_sub_type = UBMC_DEFAULT;
	char buf[UBMC_SUB_NAME_MAX];
	char prod_sub_name[UBMC_SUB_NAME_MAX];
	char len;
	FILE *fp;
	if(access(PRODUCT_SUB_PATH,F_OK) != 0)
	{
		eeprom_err("Can not find %s file :%s",PRODUCT_SUB_PATH,strerror(errno));
		return -1;
	}
	fp = fopen(PRODUCT_SUB_PATH,"r+");
	if(fp == NULL)
	{
		eeprom_err("open %s fail :%s",PRODUCT_SUB_PATH,strerror(errno));
		return -1;
	}
	fgets(buf,UBMC_SUB_NAME_MAX,fp);
	if (buf[strlen(buf)-1] == '\n')
		buf[strlen(buf)-1] = '\0';
	strcpy(prod_sub_name,buf);
	if(strcmp(prod_sub_name,UBMC_M_NAME) == 0)
	{
		ubmc_sub_type = UBMC_M;
	}
	else
	{
		ubmc_sub_type = UBMC_DEFAULT;
	}
	fclose(fp);
	return ubmc_sub_type;
}
int main(int argc, char *argv[])
{
	char * prog;
	int opt;
	int ubmc_product_sub_type;
	/*
	 *record if there is arguments or not
	 */
	int arg_flag = 0;
	int ret = 0;
	prog = argv[0];
	ubmc_product_sub_type = get_machine_prod_sub();
	if(ubmc_product_sub_type < 0)
	{
		eeprom_err("Invalid product_sub_type on %s!",PRODUCT_SUB_PATH);
		strcpy(file_path,file_path_bus0);

	}
	else if(UBMC_DEFAULT == ubmc_product_sub_type)
	{
		/*UBMC eeprom use default file*/
		strcpy(file_path,file_path_bus0);
	}
	else if(UBMC_M == ubmc_product_sub_type)
	{
		/*Marvell UBMC eeprom attached to i2c bus 1*/
		strcpy(file_path,file_path_bus1);
		//printf("%s \n",file_path);
	}
	else
	{
		strcpy(file_path,file_path_bus0);
	}
	//printf("%s ubmc_product_sub_type is %d \n",file_path,ubmc_product_sub_type);
	while ((opt = getopt(argc, argv, "rw:vh:fpc")) != -1)
	{

		arg_flag++;

		switch (opt)
		{
		case 'v':
			show_eeprom(SHOW_METHOD_VERBOSE);
			break;
		case 'r':
			show_eeprom(SHOW_METHOD_NAME_VALUE);
			break;
		case 'w':
			update_eeprom(optarg);
			break;
		case 'f':
			ret = create_eeprom_info_file(CREATE_MODEL_FILE);
		break;
		case 'p':
			ret = create_eeprom_info_file(CREATE_MODEL_BY_PN);
			if(ret)
			{
				eeprom_err("Invalid eeprom content, please contact support!");
			}
		break;
		case 'c':
			ret = check_eeprom_data();
			if(ret)
			{
				eeprom_err("Invalid eeprom content(eeprom checksum is incorrect), please contact support!");
			}
		break;
		case '?':
		case 'h':
		default:
			usage(prog);
			break;
		}
	}

	/*
	 *if there is arguments ,output usage
	 */
	if (!arg_flag)
		usage(prog);

	return ret;
}
