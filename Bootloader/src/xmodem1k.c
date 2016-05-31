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
#include <LPC11xx.h>
#include "crc.h"
#include "uart.h"
#include "xmodem1k.h"

/* Protocol control ASCII characters */
#define SOH							0x01
#define STX							0x02
#define EOT							0x04
#define ACK							0x06
#define NAK							0x15
#define POLL						0x43

/* Internal state machine */
#define STATE_IDLE					0
#define STATE_CONNECTING			1
#define STATE_RECEIVING				2

/* Define the rate at which the server will be polled when starting a transfer */
#define POLL_PERIOD_ms				3000

/* Define packet timeout period (maximum time to receive a packet) */
#define PACKET_TIMEOUT_PERIOD_ms	7000

/* Baud rate to be used by UART interface */
#define BAUD_RATE					9600

/* Size of packet payloads and header */
#define LONG_PACKET_PAYLOAD_LEN		1024
#define SHORT_PACKET_PAYLOAD_LEN	128
#define PACKET_HEADER_LEN			3

/* Buffer in which received data is stored, must be aligned on a word boundary
   as point to this array is going to be passed to IAP routines (which require
   word alignment). */
static uint8_t au8RxBuffer[LONG_PACKET_PAYLOAD_LEN] __attribute__ ((aligned(4)));

/* Local functions */
static void vTimerStart(uint32_t u32Periodms);

/*****************************************************************************
 ** Function name:
 **
 ** Descriptions:
 **
 ** Parameters:	    None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void vXmodem1k_Client(uint32_t (*pu32Xmodem1kRxPacketCallback)(uint8_t *pu8Data, uint16_t u16Len))
{
	uint32_t u32InProgress = 1;
	uint32_t u32State = STATE_IDLE;
	uint32_t u32ByteCount;
	uint32_t u32PktLen;
	uint16_t u16CRC;

	/* Prepare UART0 for RX/TX */
	vUARTInit(BAUD_RATE);

	while(u32InProgress)
	{
		switch (u32State)
		{
			case STATE_IDLE:
			{
				/* Send command to server indicating we are ready to receive */
				uint8_t u8Cmd = POLL;
				vUARTSend(&u8Cmd, 1);

				/* Start timeout to send another poll if we do not get a response */
				vTimerStart(POLL_PERIOD_ms);

				/* Wait for a response */
				u32State = STATE_CONNECTING;
			}
			break;

			case STATE_CONNECTING:
			{
				uint8_t u8Data;

				/* Check if a character has been received on the UART */
				if (u8UARTReceive(&u8Data))
				{
					/* Expecting a start of packet character */
					if ((u8Data == STX) || (u8Data == SOH))
					{
						if (u8Data == STX)
						{
							/* STX indicates long payload packet is being transmitted */
							u32PktLen = LONG_PACKET_PAYLOAD_LEN;
						}
						else
						{
							/* SOH indicates short payload packet is being transmitted */
							u32PktLen = SHORT_PACKET_PAYLOAD_LEN;
						}
						u32ByteCount = 1;

						/* Start packet timeout */
						vTimerStart(PACKET_TIMEOUT_PERIOD_ms);

						/* Wait for a further characters */
						u32State = STATE_RECEIVING;
					}
				}
				else /* No data received yet, check poll command timeout */
				{
					if ((LPC_TMR32B0->TCR & 0x01) == 0)
					{
						/* Timeout expired following poll command transmission so try again.. */
						uint8_t u8Cmd = POLL;
						vUARTSend(&u8Cmd, 1);

						/* Restart timeout to send another poll if we do not get a response */
						vTimerStart(POLL_PERIOD_ms);
					}
				}
			}
			break;

			case STATE_RECEIVING:
			{
				uint8_t u8Data;

				/* Check if a character has been received on the UART */
				if (u8UARTReceive(&u8Data))
				{
					/* Position of received byte determines action we take */
					if (u32ByteCount == 0)
					{
						/* Expecting a start of packet character */
						if ((u8Data == STX) || (u8Data == SOH))
						{
							if (u8Data == STX)
							{
								/* STX indicates long payload packet is being transmitted */
								u32PktLen = LONG_PACKET_PAYLOAD_LEN;
							}
							else
							{
								/* SOH indicates short payload packet is being transmitted */
								u32PktLen = SHORT_PACKET_PAYLOAD_LEN;
							}
							u32ByteCount = 1;

							/* Start packet timeout */
							vTimerStart(PACKET_TIMEOUT_PERIOD_ms);
						}
						else if (u8Data == EOT)
						{
							/* Server indicating transmission is complete */
							uint8_t u8Cmd = ACK;
							vUARTSend(&u8Cmd, 1);

							/* Close xmodem client */
							u32InProgress = 0;
						}
						else
						{
							/* TODO - Unexpected character, do what...? */
						}
					}
					else if (u32ByteCount == 1)
					{
						/* Byte 1 is the packet number - should be different from last one we received */
						u32ByteCount++;
					}
					else if (u32ByteCount == 2)
					{
						/* Byte 2 is the packet number inverted - check for error with last byte */
						u32ByteCount++;
					}
					else if (((u32ByteCount == 131 ) && (u32PktLen == SHORT_PACKET_PAYLOAD_LEN)) ||
							 ((u32ByteCount == 1027) && (u32PktLen == LONG_PACKET_PAYLOAD_LEN)))
					{
						/* If payload is short byte 131 is the MS byte of the packet CRC, if payload
						   is long byte 1027 is the MS byte of the packet CRC. */
						u16CRC = u8Data;
						u32ByteCount++;
					}
					else if (((u32ByteCount == 132)  && (u32PktLen == SHORT_PACKET_PAYLOAD_LEN)) ||
					         ((u32ByteCount == 1028) && (u32PktLen == LONG_PACKET_PAYLOAD_LEN)))
					{
						/* If payload is short byte 132 is the LS byte of the packet CRC, if payload
						   is long byte 1028 is the LS byte of the packet CRC. */
						u16CRC <<= 8;
						u16CRC  |= u8Data;

						/* Check the received CRC against the CRC we generate on the packet data */
						if (u16CRC_Calc16(&au8RxBuffer[0], u32PktLen) == u16CRC)
						{
							uint8_t u8Cmd;

							/* Have now received full packet, call handler BEFORE sending ACK to application
							   can process data before more is sent. */
							if (pu32Xmodem1kRxPacketCallback(&au8RxBuffer[0], u32PktLen) != 0)
							{
								/* Packet handled successfully, send ACK to server indicating we are ready for next packet */
								u8Cmd = ACK;
								vUARTSend(&u8Cmd, 1);
							}
							else
							{
								/* Something went wrong with packet handler, all we can do is send NAK causing the
								   packet to be retransmitted by the server.. */
								u8Cmd = NAK;
								vUARTSend(&u8Cmd, 1);
							}
						}
						else /* Error CRC calculated does not match that received */
						{
							/* Indicate problem to server - should result in packet being resent.. */
							uint8_t u8Cmd = NAK;
							vUARTSend(&u8Cmd, 1);
						}
						u32ByteCount = 0;
					}
					else
					{
						/* Must be payload data so store */
						au8RxBuffer[u32ByteCount - PACKET_HEADER_LEN] = u8Data;
						u32ByteCount++;
					}
				}
				else
				{
					/* TODO - Check packet timeout */
				}
			}
			break;

			default:
				break;
		}
	}
}

/*****************************************************************************
 ** Function name:
 **
 ** Descriptions:
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
static void vTimerStart(uint32_t u32Periodms)
{
	/* Enable the timer clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL << 9);

	/* Configure the timer so that we can poll for a match */
	LPC_TMR32B0->TCR = 0x02;		/* reset timer */
	LPC_TMR32B0->PR  = 0x00;		/* set prescaler to zero */
	LPC_TMR32B0->MR0 = u32Periodms * ((SystemCoreClock / (LPC_TMR32B0->PR + 1)) / 1000UL);
	LPC_TMR32B0->IR  = 0xFF;		/* reset all interrupts */
	LPC_TMR32B0->MCR = 0x04;		/* stop timer on match */
	LPC_TMR32B0->TCR = 0x01;		/* start timer */
}

/*****************************************************************************
 **                            End Of File
 *****************************************************************************/
