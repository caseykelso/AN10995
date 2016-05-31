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
#include <LPC11xx.h>
#include "uart.h"

/* UART line status register (LSR) bit definitions */
#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

/*****************************************************************************
** Function name:	vUARTInit
**
** Descriptions:	Initialize UART0 port, setup pin select, clock, parity,
**                  stop bits, FIFO, etc.
**
** Parameters:		u32BaudRate - UART baudrate
**
** Returned value:	None
**
*****************************************************************************/
void vUARTInit(uint32_t u32BaudRate)
{
	uint32_t Fdiv;
	uint32_t regVal;

	/* Not using interrupts */
	NVIC_DisableIRQ(UART_IRQn);

	/* UART I/O config */
	LPC_IOCON->PIO1_6 &= ~0x07;
	LPC_IOCON->PIO1_6 |= 0x01;     /* UART RXD */
	LPC_IOCON->PIO1_7 &= ~0x07;
	LPC_IOCON->PIO1_7 |= 0x01;     /* UART TXD */

	/* Enable UART clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
	LPC_SYSCON->UARTCLKDIV = 0x1;     /* divided by 1 */

	LPC_UART->LCR = 0x83;             /* 8 bits, no Parity, 1 Stop bit */
	regVal = LPC_SYSCON->UARTCLKDIV;
	Fdiv = (((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/regVal)/16)/u32BaudRate ;	/*baud rate */

	LPC_UART->DLM = Fdiv / 256;
	LPC_UART->DLL = Fdiv % 256;
	LPC_UART->LCR = 0x03;		/* DLAB = 0 */
	LPC_UART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

	/* Read to clear the line status. */
	regVal = LPC_UART->LSR;

	/* Ensure a clean start, no data in either TX or RX FIFO. */
	while ( (LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );

	while ( LPC_UART->LSR & LSR_RDR )
	{
		regVal = LPC_UART->RBR;	/* Dump data from RX FIFO */
	}
}

/*****************************************************************************
** Function name:	vUARTReceive
**
** Descriptions:	Reads received data from UART0 FIFO
**
** Parameters:		pu8Buffer - Pointer to buffer in which received characters
** 					are to be stored.
**
** Returned value:	Number of character read out of receive FIFO.
**
*****************************************************************************/
uint8_t u8UARTReceive(uint8_t *pu8Buffer)
{
	uint8_t u8Len = 0;

	if (LPC_UART->LSR & LSR_RDR)
	{
		*pu8Buffer = LPC_UART->RBR;
		u8Len++;
	}
	return u8Len;
}

/*****************************************************************************
** Function name:	vUARTSend
**
** Descriptions:	Send a block of data to the UART0.
**
** parameters:		pu8Buffer - Pointer to buffer containing data to be sent.
** 					u32Len - Number of bytes to send.
**
** Returned value:	None
**
*****************************************************************************/
void vUARTSend(uint8_t *pu8Buffer, uint32_t u32Len)
{
	while ( u32Len != 0 )
	{
		/* Send character to UART */
		LPC_UART->THR = *pu8Buffer;
		/* Wait until transmission is complete */
		while ((LPC_UART->LSR & LSR_TEMT) == 0);
		pu8Buffer++;
		u32Len--;
	}
}

/******************************************************************************
**                            End Of File
******************************************************************************/
