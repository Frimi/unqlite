/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                         STM32F746G-DISCO
*                                         Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : DC
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include "fs_dev_nor_w25q.h"
#include "debug.h"

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

#define  APP_CFG_FS_EN                         DEF_ENABLED


/*
*********************************************************************************************************
*                                          FS CONFIGURATION
*
* Note(s) : This section define various prerprocessor constant used by the example initialization code
*           located in fs_app.c to configure the file system.
*********************************************************************************************************
*/

#define  APP_CFG_FS_DEV_CNT                     1u              /* Maximum number of opened devices.                    */
#define  APP_CFG_FS_VOL_CNT                     1u              /* Maximum number of opened volumes.                    */
#define  APP_CFG_FS_FILE_CNT                    10u             /* Maximum number of opened files.                      */
#define  APP_CFG_FS_DIR_CNT                     1u              /* Maximum number of opened directories.                */
#define  APP_CFG_FS_BUF_CNT         (4u * APP_CFG_FS_VOL_CNT)   /* Internal buffer count.                               */
#define  APP_CFG_FS_DEV_DRV_CNT                 1u              /* Maximum number of different device drivers.          */
#define  APP_CFG_FS_WORKING_DIR_CNT             1u              /* Maximum number of active working directories.        */
#define  APP_CFG_FS_MAX_SEC_SIZE                4096u           /* Maximum sector size supported.                       */

#define  APP_CFG_FS_IDE_EN                      DEF_DISABLED    /* Enable/disable the IDE\CF initialization.            */
#define  APP_CFG_FS_MSC_EN                      DEF_DISABLED    /* Enable/disable the MSC initialization.               */
#define  APP_CFG_FS_NAND_EN                     DEF_DISABLED    /* Enable/disable the NAND initialization.              */
#define  APP_CFG_FS_NOR_EN                      DEF_ENABLED     /* Enable/disable the NOR initialization.               */
#define  APP_CFG_FS_RAM_EN                      DEF_DISABLED    /* Enable/disable the RAMDisk initialization.           */
#define  APP_CFG_FS_SD_EN                       DEF_DISABLED    /* Enable/disable the SD (SPI) initialization.          */
#define  APP_CFG_FS_SD_CARD_EN                  DEF_DISABLED    /* Enable/disable the SD (Card) initialization.         */

/*
*********************************************************************************************************
*                                    RAMDISK DRIVER CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_FS_RAM_NBR_SECS     78                       /* RAMDisk size in sectors.                             */
#define  APP_CFG_FS_RAM_SEC_SIZE     512                       /* RAMDisk sector size in byte.                         */


/*
*********************************************************************************************************
*                                      NOR DRIVER CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_FS_NOR_ADDR_BASE           0x00000000u         /* Base address of flash.                               */
#define  APP_CFG_FS_NOR_REGION_NBR          0u                  /* Block region within flash.                           */
#define  APP_CFG_FS_NOR_ADDR_START          0x00000000u         /* Start address of block region within NOR.            */
#define  APP_CFG_FS_NOR_DEV_SIZE            0x01000000u         /* Device size in bytes.  128 Mbit =  16 Mbyte.         */
#define  APP_CFG_FS_NOR_SEC_SIZE            4096u               /* Sector size.                                         */
#define  APP_CFG_FS_NOR_PCT_RSVD            5u                  /* Reserved area in percent.                            */
#define  APP_CFG_FS_NOR_ERASE_CNT_DIFF_TH   5u                  /* Erase count difference threshold.                    */
#define  APP_CFG_FS_NOR_PHY_PTR          	&FSDev_NOR_W25Q 	/* PHY pointer.                                         */
#define  APP_CFG_FS_NOR_BUS_WIDTH           8u                  /* Bus width.                                           */
#define  APP_CFG_FS_NOR_BUS_WIDTH_MAX       8u                  /* Maximum bus width.                                   */
#define  APP_CFG_FS_NOR_PHY_DEV_CNT         1u                  /* Number of interleaved devices.                       */
#define  APP_CFG_FS_NOR_MAX_CLK_FREQ        95000000u           /* Maximum clock frequency for serial flash.            */

                                                                /* ------------- MICRON N25Q ADDTL CONFIG ------------- */
#define  APP_CFG_FS_NOR_QUAD_IO_MODE        DEF_ENABLED         /* Enable OR Disable Quad I/O Mode.                     */
#define  APP_CFG_FS_NOR_DUMMY_CYCLE_CNT     8u                  /* Number of Dummy Clock Cycles. Affects Clk Freq.      */

/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                        0u
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                       1u
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                        2u
#endif

#define  APP_TRACE_LEVEL                        TRACE_LEVEL_DBG
#define  APP_TRACE                              DEBUG_printfNoLF

#define  APP_TRACE_INFO(x)               ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_TRACE x) : (void)0)

#endif
