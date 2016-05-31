/*****************************************************************************
 * $Id$
 *
 * Project: NXP LPC1100 Secondary Bootloader Example
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
#include "crc.h"

/*****************************************************************************
** Function name:	u16CRC_Calc16
**
** Descriptions:	Calculate 16-bit CRC value used by Xmodem-1K protocol.
**
** Parameters:	    pu8Data - Pointer to buffer containing 8-bit data values.
** 					i16Len - Number of 8-bit values to be included in calculation.
**
** Returned value:  16-bit CRC
**
******************************************************************************/
uint16_t u16CRC_Calc16(const uint8_t *pu8Data, int16_t i16Len)
{
	uint8_t i;
	uint16_t u16CRC = 0;

    while(--i16Len >= 0)
    {
    	i = 8;
    	u16CRC = u16CRC ^ (((uint16_t)*pu8Data++) << 8);

    	do
        {
    		if (u16CRC & 0x8000)
    		{
    			u16CRC = u16CRC << 1 ^ 0x1021;
    		}
    		else
    		{
    			u16CRC = u16CRC << 1;
    		}
        }
    	while(--i);
    }
    return u16CRC;
}

/*****************************************************************************
**                            End Of File
******************************************************************************/
