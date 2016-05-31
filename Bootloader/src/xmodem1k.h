/*****************************************************************************
 * $Id$
 *
 * Project: 	NXP LPC1100 Secondary Bootloader Example
 *
 * Description: Implements an Xmodem1K client (receiver).
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
#ifndef __XMODEM1K_H
#define __XMODEM1K_H

#include <stdint.h>

void vXmodem1k_Client(uint32_t (*pu32Xmodem1kRxPacketCallback)(uint8_t *pu8Data, uint16_t u16Len));

#endif /* end __XMODEM1K_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
