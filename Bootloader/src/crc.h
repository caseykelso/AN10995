/*****************************************************************************
 * $Id$
 *
 * Project: 	NXP LPC1100 Secondary Bootloader Example
 *
 * Description: Provides routine that calculates the 16-bit CRC value used by
 *              the xmodem-1k protocol.
 *
 * Copyright(C) 2010, NXP Semiconductor
 * All rights reserved.
 *
 *****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 *****************************************************************************/
#ifndef __CRC_H
#define __CRC_H

#include <stdint.h>

uint16_t u16CRC_Calc16(const uint8_t *pu8Data, int16_t u16Len);

#endif /* end __CRC_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
