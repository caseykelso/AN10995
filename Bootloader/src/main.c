/*****************************************************************************
 * $Id$
 *
 * Project: 	NXP LPC1100 Secondary Bootloader Example
 *
 * Description:	Secondary bootloader that permanently resides in sector zero
 * 				flash memory. Uses UART0 and the XMODEM 1K protocol to load
 * 				an new application into sectors 1 onwards. Also redirects
 * 				interrupts to the functions contained in the application
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
#include "crc.h"
#include "xmodem1k.h"

/* Define flash memory address at which user application is located */
#define APP_START_ADDR						0x00001000UL
#define APP_END_ADDR						0x00008000UL

/* Define the flash sectors used by the application */
#define APP_START_SECTOR					1
#define APP_END_SECTOR						7

/* Define location in flash memory that contains the application valid check value */
#define APP_VALID_CHECK_ADDR				0x00007FFCUL

/* Prototypes for functions that re-direct interrupts to handlers specified
   in application vector table. Implemented as naked functions as they do not
   need to store anything on the stack, they simply change the value of the
   program counter. */
void NMI_Handler(void) __attribute__ (( naked ));
void HardFault_Handler(void) __attribute__ (( naked ));
void SVCall_Handler(void) __attribute__ (( naked ));
void PendSV_Handler(void) __attribute__ (( naked ));
void SysTick_Handler(void) __attribute__ (( naked ));
void WAKEUP_IRQHandler(void) __attribute__ (( naked ));
void I2C_IRQHandler(void) __attribute__ (( naked ));
void TIMER16_0_IRQHandler(void) __attribute__ (( naked ));
void TIMER16_1_IRQHandler(void) __attribute__ (( naked ));
void TIMER32_0_IRQHandler(void) __attribute__ (( naked ));
void TIMER32_1_IRQHandler(void) __attribute__ (( naked ));
void SSP_IRQHandler(void) __attribute__ (( naked ));
void UART_IRQHandler(void) __attribute__ (( naked ));
void USB_IRQHandler(void) __attribute__ (( naked ));
void USB_FIQHandler(void) __attribute__ (( naked ));
void ADC_IRQHandler(void) __attribute__ (( naked ));
void WDT_IRQHandler(void) __attribute__ (( naked ));
void BOD_IRQHandler(void) __attribute__ (( naked ));
void FMC_IRQHandler(void) __attribute__ (( naked ));
void PIOINT3_IRQHandler(void) __attribute__ (( naked ));
void PIOINT2_IRQHandler(void) __attribute__ (( naked ));
void PIOINT1_IRQHandler(void) __attribute__ (( naked ));
void PIOINT0_IRQHandler(void) __attribute__ (( naked ));

static void vBootLoader_Task(void);
static uint32_t u32BootLoader_AppPresent(void);
static uint32_t u32Bootloader_WriteCRC(uint16_t u16CRC);
static uint32_t u32BootLoader_ProgramFlash(uint8_t *pu8Data, uint16_t u16Len);

/*****************************************************************************
 ** Function name:  main
 **
 ** Description:	Bootloader control loop.
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

	/* Verify if a valid user application is present in the upper sectors
	   of flash memory. */
	if (u32BootLoader_AppPresent() == 0)
	{
		/* Valid application not present, execute bootloader task that will
		   obtain a new application and program it to flash.. */
		vBootLoader_Task();

		/* Above function only returns when new application image has been
		   successfully programmed into flash. Begin execution of this new
		   application by resetting the device. */
		NVIC_SystemReset();
	}
	else
	{
		/* Valid application located in the next sector(s) of flash so execute */

		/* Load main stack pointer with application stack pointer initial value,
		   stored at first location of application area */
		asm volatile("ldr r0, =0x1000");
		asm volatile("ldr r0, [r0]");
		asm volatile("mov sp, r0");

		/* Load program counter with application reset vector address, located at
		   second word of application area. */
		asm volatile("ldr r0, =0x1004");
		asm volatile("ldr r0, [r0]");
		asm volatile("mov pc, r0");

		/* User application execution should now start and never return here.... */
	}
	/* This should never be executed.. */
	return 0;
}

/*****************************************************************************
 ** Function name:  vBootLoader_Task
 **
 ** Description:	Erases application flash area and starts XMODEM client so
 ** 				that new application can be downloaded.
 **
 ** Parameters:	    None
 **
 ** Returned value: None
 **
 *****************************************************************************/
static void vBootLoader_Task(void)
{
	/* Erase the application flash area so it is ready to be reprogrammed with the new application */
	if (u32IAP_PrepareSectors(APP_START_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
	{
		if (u32IAP_EraseSectors(APP_START_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
		{
			uint16_t u16CRC = 0;

			/* Start the xmodem client, this function only returns when a
			   transfer is complete. Pass it pointer to function that will
			   handle received data packets */
			vXmodem1k_Client(&u32BootLoader_ProgramFlash);

			/* Programming is now complete, calculate the CRC of the flash image */
			u16CRC = u16CRC_Calc16((const uint8_t *)APP_START_ADDR, (APP_END_ADDR - APP_START_ADDR - 4));

			/* Write the CRC value into the last 16-bit location of flash, this
			   will be used to check for a valid application at startup  */
			(void)u32Bootloader_WriteCRC(u16CRC);
		}
	}
}

/*****************************************************************************
 ** Function name:	u32Bootloader_WriteCRC
 **
 ** Description:	Writes a 16-bit CRC value to the last location in flash
 ** 				memory, the bootloader uses this value to check for a valid
 ** 				application at startup.
 **
 ** Parameters:	    u16CRC - CRC value to be written to flash
 **
 ** Returned value: 1 if CRC written to flash successfully, otherwise 0.
 **
 *****************************************************************************/
static uint32_t u32Bootloader_WriteCRC(uint16_t u16CRC)
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

	/* Set the CRC value to be written back */
	a32DummyData[IAP_FLASH_PAGE_SIZE_WORDS - 1] = (uint32_t)u16CRC;

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
 ** Function name:	u32BootLoader_ProgramFlash
 **
 ** Description:
 **
 ** Parameters:	    None
 **
 ** Returned value: 0 if programming failed, otherwise 1.
 **
 *****************************************************************************/
static uint32_t u32BootLoader_ProgramFlash(uint8_t *pu8Data, uint16_t u16Len)
{
	uint32_t u32Result = 0;

	static uint32_t u32NextFlashWriteAddr = APP_START_ADDR;

	if ((pu8Data != 0) && (u16Len != 0))
	{
		/* Prepare the flash application sectors for reprogramming */
		if (u32IAP_PrepareSectors(APP_START_SECTOR, APP_END_SECTOR) == IAP_STA_CMD_SUCCESS)
		{
			/* Ensure that amount of data written to flash is at minimum the
			   size of a flash page */
			if (u16Len < IAP_FLASH_PAGE_SIZE_BYTES)
			{
				u16Len = IAP_FLASH_PAGE_SIZE_BYTES;
			}

			/* Write the data to flash */
			if (u32IAP_CopyRAMToFlash(u32NextFlashWriteAddr, (uint32_t)pu8Data, u16Len) == IAP_STA_CMD_SUCCESS)
			{
				/* Check that the write was successful */
				if (u32IAP_Compare(u32NextFlashWriteAddr, (uint32_t)pu8Data, u16Len, 0) == IAP_STA_CMD_SUCCESS)
				{
					/* Write was successful */
					u32NextFlashWriteAddr += u16Len;
					u32Result = 1;
				}
			}
		}
	}
	return (u32Result);
}

/*****************************************************************************
 ** Function name:  u32BootLoader_AppPresent
 **
 ** Description:	Checks if an application is present by comparing CRC of
 ** 				flash contents with value present at last location in flash.
 **
 ** Parameters:	    None
 **
 ** Returned value: 1 if application present, otherwise 0.
 **
 *****************************************************************************/
static uint32_t u32BootLoader_AppPresent(void)
{
	uint16_t u16CRC = 0;
	uint32_t u32AppPresent = 0;
	uint16_t *pu16AppCRC = (uint16_t *)(APP_END_ADDR - 4);

	/* Check if a CRC value is present in application flash area */
	if (*pu16AppCRC != 0xFFFFUL)
	{
		/* Memory occupied by application CRC is not blank so calculate CRC of
		   image in application area of flash memory, and check against this
		   CRC.. */
		u16CRC = u16CRC_Calc16((const uint8_t *)APP_START_ADDR, (APP_END_ADDR - APP_START_ADDR - 4));

		if (*pu16AppCRC == u16CRC)
		{
			u32AppPresent = 1;
		}
	}
	return u32AppPresent;
}

/*****************************************************************************
 *
 *                      Interrupt redirection functions
 *
 *****************************************************************************/
/*****************************************************************************
 ** Function name:   NMI_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void NMI_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1008");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   HardFault_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void HardFault_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x100C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   SVCall_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void SVCall_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x102C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PendSV_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 ******************************************************************************/
void PendSV_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1038");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   SysTick_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void SysTick_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x103C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   WAKEUP_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 ******************************************************************************/
void WAKEUP_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1040");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   I2C_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void I2C_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x107C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER16_0_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER16_0_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1080");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER16_1_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER16_1_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1084");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER32_0_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER32_0_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1088");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER32_1_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER32_1_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x108C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   SSP_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void SSP_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1090");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   UART_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void UART_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1094");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   USB_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void USB_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x1098");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   USB_FIQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void USB_FIQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x109C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   ADC_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void ADC_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10A0");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   WDT_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void WDT_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10A4");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   BOD_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void BOD_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10A8");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   FMC_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void FMC_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10AC");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT3_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT3_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10B0");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT2_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT2_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10B4");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT1_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT1_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10B8");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT0_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT0_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x10BC");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 **                            End Of File
 *****************************************************************************/
