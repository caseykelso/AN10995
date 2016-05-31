/* Bootloader only uses the first 4k sector of flash memory */

MEMORY
{
  /* Define each memory region */
  MFlash32 (rx) : ORIGIN = 0x0, LENGTH = 0x1000 /* 4k */
  RamLoc8 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x1FE0 /* 8k less 32 bytes used for IAP */

}
  /* Define a symbol for the top of each memory region */
  __top_MFlash32 = 0x0 + 0x1000;
  __top_RamLoc8 = 0x10000000 + 0x1FE0;
