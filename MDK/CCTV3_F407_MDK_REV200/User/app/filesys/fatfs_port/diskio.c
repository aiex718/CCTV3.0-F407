/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h" /* Declarations of disk functions */
#include "ff.h"		/* Obtains integer types */

#include "bsp/platform/device/dev_disk.h"
/* Definitions of physical drive number for each drive */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = RES_ERROR;

	if (pdrv >= Disk_GetNums(Dev_Disk_list))
		stat = RES_PARERR;
	else if (Disk_IsReady(Dev_Disk_list[pdrv]) == false)
		stat = RES_NOTRDY;
	else
		stat = RES_OK;

	return stat;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = RES_ERROR;

	if (pdrv >= Disk_GetNums(Dev_Disk_list))
		stat = RES_PARERR;
	else
	{
		Disk_t *disk = Dev_Disk_list[pdrv];
		if (disk->Init(disk) == false)
			stat = STA_NOINIT;
		else
			stat = RES_OK;
	}

	return stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	DRESULT stat = RES_ERROR;

	if (pdrv >= Disk_GetNums(Dev_Disk_list))
		stat = RES_PARERR;
	else
	{
		Disk_t *disk = Dev_Disk_list[pdrv];
		sector *= Dev_Disk_Flash->block_size;
		count *= Dev_Disk_Flash->block_size;

		if (disk->Read(disk, buff, sector, count) == false)
			stat = RES_ERROR;
		else
			stat = RES_OK;
	}

	return stat;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	DRESULT stat = RES_ERROR;

	if (pdrv >= Disk_GetNums(Dev_Disk_list))
		stat = RES_PARERR;
	else
	{
		Disk_t *disk = Dev_Disk_list[pdrv];
		sector *= Dev_Disk_Flash->block_size;
		count *= Dev_Disk_Flash->block_size;

		if (disk->Write(disk, (uint8_t*)buff, sector, count) == false)
			stat = RES_ERROR;
		else
			stat = RES_OK;
	}

	return stat;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE cmd,  /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	DRESULT stat = RES_ERROR;

	if (pdrv >= Disk_GetNums(Dev_Disk_list))
		stat = RES_PARERR;
	else
	{
		Disk_t *disk = Dev_Disk_list[pdrv];
		uint32_t block_num, block_size;
		Disk_GetCapacity(disk, &block_num, &block_size);

		switch (cmd)
		{
		case GET_SECTOR_COUNT:
			*(DWORD *)buff = block_num;
			stat = RES_OK;
			break;

		case GET_SECTOR_SIZE:
			*(DWORD *)buff = block_size;
			stat = RES_OK;
			break;

		case GET_BLOCK_SIZE:
			*(DWORD *)buff = 1;
			stat = RES_OK;
			break;

		case CTRL_SYNC:
			stat = RES_OK;
			break;

		default:
			stat = RES_PARERR;
			break;
		}
	}
	return stat;
}
