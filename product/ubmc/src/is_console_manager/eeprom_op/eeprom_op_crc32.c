/*************************************************************************
 > File Name: eeprom_op_crc32.c
 > Author: allen
 > Mail: allen.zhou@net-perf.com
 > Created Time: Wed 11 Jul 2018 02:52:58 PM HKT
 ************************************************************************/
/*
 crc32 ported from uboot in uboot/lib/crc32.c
 */
#include<stdio.h>
#include<stdint.h>

#define DO_CRC(x) crc = tab[(crc ^ (x)) & 255] ^ (crc >> 8)

static uint32_t crc_table[256];
/*
 Generate a table for a byte-wise 32-bit CRC calculation on the polynomial:
 x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

 Polynomials over GF(2) are represented in binary, one bit per coefficient,
 with the lowest powers in the most significant bit.  Then adding polynomials
 is just exclusive-or, and multiplying a polynomial by x is a right shift by
 one.  If we call the above polynomial p, and represent a byte as the
 polynomial q, also with the lowest power in the most significant bit (so the
 byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is (q*x^32) mod p,
 where a mod b means the remainder after dividing a by b.

 This calculation is done using the shift-register method of multiplying and
 taking the remainder.  The register is initialized to zero, and for each
 incoming bit, x^32 is added mod p to the register if the bit is a one (where
 x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
 x (which is shifting right by one and adding x^32 mod p if the bit shifted
 out is a one).  We start with the highest power (least significant bit) of
 q and repeat for all eight bits of q.

 The table is simply the CRC of all possible eight bit values.  This is all
 the information needed to generate CRC's on data a byte at a time for all
 combinations of CRC register values and incoming bytes.
 */
void make_crc_table()
{
	uint32_t c;
	int n, k;
	unsigned long poly; /* polynomial exclusive-or pattern */
	/* terms of polynomial defining this crc (except x^32): */
	static const char p[] =
	{ 0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26 };

	/* make exclusive-or pattern from polynomial (0xedb88320L) */
	poly = 0L;
	for (n = 0; n < sizeof(p) / sizeof(char); n++)
		poly |= 1L << (31 - p[n]);

	for (n = 0; n < 256; n++)
	{
		c = (unsigned long) n;
		for (k = 0; k < 8; k++)
			c = c & 1 ? poly ^ (c >> 1) : c >> 1;
		crc_table[n] = c;
	}
}

static uint32_t crc32_no_comp(uint32_t crc, const char *buf, uint32_t len)
{
	const uint32_t *tab = crc_table;
	const uint32_t *b = (const uint32_t *) buf;
	size_t rem_len;

	make_crc_table();

	//arm cpu use little endian,so no need convert
	//crc = cpu_to_le32(crc);
	/* Align it */
	if (((long) b) & 3 && len)
	{
		uint8_t *p = (uint8_t *) b;
		do
		{
			DO_CRC(*p++);
		} while ((--len) && ((long) p) & 3);
		b = (uint32_t *) p;
	}

	rem_len = len & 3;
	len = len >> 2;
	for (--b; len; --len)
	{
		/* load data 32 bits wide, xor data 32 bits wide. */
		crc ^= *++b; /* use pre increment for speed */
		DO_CRC(0);
		DO_CRC(0);
		DO_CRC(0);
		DO_CRC(0);
	}
	len = rem_len;
	/* And the last few bytes */
	if (len)
	{
		uint8_t *p = (uint8_t *) (b + 1) - 1;
		do
		{
			DO_CRC(*++p); /* use pre increment for speed */
		} while (--len);
	}

	return crc;
	//arm cpu use little endian,so no need convert
	//return le32_to_cpu(crc);
}

uint32_t crc32(uint32_t crc, const char *p, uint32_t len)
{
	return crc32_no_comp(crc ^ 0xffffffffL, p, len) ^ 0xffffffffL;
}

