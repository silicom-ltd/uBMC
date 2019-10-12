/*
 * ubmc_mgmtd_common.h
 *
 *  Created on: Oct 6, 2019
 *      Author: jeff_zheng
 */

#ifndef INC_UBMC_MGMTD_COMMON_H_
#define INC_UBMC_MGMTD_COMMON_H_

#define UBMC_MGMTD_VENDOR_LIST	"UBMC_EVAL", "MANUFACTURE", "ATT"
#define UBMC_PRODUCT_ID	1
#define	UBMC_PRODUCT_NAME "UBMC"

#define	VENDOR_UBMC_EVAL		((UBMC_PRODUCT_ID << 4)|0x1)
#define	VENDOR_UBMC_MANUFACTURE	((UBMC_PRODUCT_ID << 4)|0x2)
#define	VENDOR_UBMC_ATT			((UBMC_PRODUCT_ID << 4)|0x3)


#endif /* INC_UBMC_MGMTD_COMMON_H_ */
