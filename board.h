#ifndef __BOARD_H__
#define __BOARD_H__

#include "dram.h"
#include "sunxi_usart.h"
#include "sunxi_sdhci.h"

#define USART_DBG usart1_dbg

#define CONFIG_BOOT_SDCARD
#define CONFIG_BOOT_MMC

#define CONFIG_FATFS_CACHE_SIZE		 (CONFIG_DTB_LOAD_ADDR - SDRAM_BASE) // in bytes
#define CONFIG_SDMMC_SPEED_TEST_SIZE 1024 // (unit: 512B sectors)

#define CONFIG_CPU_FREQ 1200000000

// #define CONFIG_ENABLE_CPU_FREQ_DUMP

#define CONFIG_KERNEL_FILENAME "0:zImage"
#define CONFIG_DTB_FILENAME	   "0:sun8i-t113s-dewarmte.dtb"
//#define CONFIG_DTB_FILENAME	   "0:sun8i-t113s-upatch.dtb"
//#define CONFIG_DTB_FILENAME	   "0:sun8i-t113-mangopi-dual.dtb"

#define CONFIG_KERNEL_LOAD_ADDR (SDRAM_BASE + (72 * 1024 * 1024))
#define CONFIG_DTB_LOAD_ADDR	(SDRAM_BASE + (64 * 1024 * 1024))

#define RESTORE_FEL_ON_FAIL
#define RESTORE_FEL_MAX_FAIL 32

extern dram_para_t	 ddr_param;
extern sunxi_usart_t	 USART_DBG;

extern void board_init(void);

#endif
