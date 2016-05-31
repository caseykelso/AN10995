/*****************************************************************************
 * $Id$
 *
 * Project: 	NXP LPC1100 Secondary Bootloader Example
 *
 * Description: Basic application that is designed to be programmed into flash
 *              along with the secondary bootloader. It flashes an LED, using
 *              an interrupt driven timer to control the period, demonstrating
 *              that interrupts are correctly routed to the application by the
 *              bootloader.
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
#include "IAP.h"

/* Define flash memory address at which user application is located */
#define APP_START_ADDR						0x00001000UL
#define APP_END_ADDR						0x00008000UL

/* Define the flash sectors used by the application */
#define APP_START_SECTOR					1
#define APP_END_SECTOR						7

void SysTick_Handler(void);

static uint32_t u32InvalidateApp(void);
static uint8_t u8LedOn = 0;

/*****************************************************************************
 ** Function name:  main
 **
 ** Description:	Simply flashes an LED uses the system tick timer.
 **
 ** Parameters:	    None
 **
 ** Returned value: None
 **
 *****************************************************************************/
int main(void)
{
	/* Basic chip initialization is taken care of in SystemInit() called
	   from the startup code. SystemInit() and chip settings are defined
	   in the CMSIS system_<part family>.c file. */

	/* Initialize the GPIO (P0.7) that will be used to control the LED */
	LPC_GPIO0->DIR |= (1UL << 7);

	/* Initialize the GPIO (P0.1) that will be used to initiate an application
	   download using the secondary bootloader */
	LPC_GPIO0->DIR &= ~(1UL << 1);

	/* Setup SysTick Timer for 1 second interrupt  */
	SysTick_Config(SystemCoreClock / 100000000UL);

	while(1)
	{
		/* Check if P0.1 is being held low, if so user is requesting an
		   application download is initiated using the secondary bootloader */
		if ((LPC_GPIO0->DATA & (1UL << 1)) == 0)
		{
			/* Invalidate application CRC so that when device is reset the
			   bootloader detects no valid application is present and attempts
			   to download a new one. */
			if (u32InvalidateApp() != 0)
			{
				/* Restart and bootloader will begin */
				NVIC_SystemReset();
			}
		}
	}
	return 0 ;
}

/*****************************************************************************
 ** Function name:  SysTick_Handler
 **
 ** Description:	Toggle the state of the GPIO connected to the on-board LED
 **
 ** Parameters:	    None
 **
 ** Returned value: None
 **
 *****************************************************************************/
void SysTick_Handler(void)
{
	if (u8LedOn)
	{
		LPC_GPIO0->DATA |= (1UL << 7); /* Turn LED on */
	}
	else
	{
		LPC_GPIO0->DATA &= ~(1UL << 7); /* Turn LED off */
	}
	u8LedOn = !u8LedOn;
}

/*****************************************************************************
 ** Function name:  u32InvalidateApp
 **
 ** Description:	Corrupt the application CRC value that is written into the
 ** 				last location of flash by the bootloader.
 **
 ** Parameters:	    None
 **
 ** Returned value: Zero if unsuccessful, otherwise 1
 **
 *****************************************************************************/
uint32_t u32InvalidateApp(void)
{
	uint32_t i;
	uint32_t u32Result = 0;
	uint32_t a32DummyData[IAP_FLASH_PAGE_SIZE_WORDS];
	uint32_t *pu32Mem = (uint32_t *)(APP_END_ADDR - IAP_FLASH_PAGE_SIZE_BYTES);

	/* First copy the data that is currently present in the last page of
	   flash into a temporary buffer */
	for (i = 0 ; i < IAP_FLASH_PAGE_SIZE_WORDS; i++)
	{
		a32DummyData[i] = *pu32Mem++;
	}

	/* Set the CRC value to be written back, corrupt by setting all ones to zeros */
	a32DummyData[IAP_FLASH_PAGE_SIZE_WORDS - 1] = 0;

	if (u32IAP_PrepareSectors(APP_END_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
	{
		/* Now write the data back, only the CRC bits have changed */
		if (u32IAP_CopyRAMToFlash((APP_END_ADDR - IAP_FLASH_PAGE_SIZE_BYTES),
				                  (uint32_t)a32DummyData,
				                  IAP_FLASH_PAGE_SIZE_BYTES) == IAP_STA_CMD_SUCCESS)
		{
			u32Result = 1;
		}
	}
	return (u32Result);
}
/*****************************************************************************
 **                            End Of File
 *****************************************************************************/
