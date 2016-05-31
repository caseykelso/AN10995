/*****************************************************************************
 * $Id$
 *
 * Project: 	NXP LPC1100 Secondary Bootloader Example
 *
 * Description: Provides functions that allow communications using UART0.
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
#ifndef __UART_H 
#define __UART_H

#include <stdint.h>

void vUARTInit(uint32_t u32BaudRate);
uint8_t u8UARTReceive(uint8_t *pu8Buffer);
void vUARTSend(uint8_t *pu8Buffer, uint32_t u32Len);

#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
