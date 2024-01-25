#include "main.h"
#include "fdt.h"
#include "ff.h"
#include "sunxi_gpio.h"
#include "sunxi_sdhci.h"
#include "sunxi_spi.h"
#include "sunxi_clk.h"
#include "sunxi_dma.h"
#include "sdmmc.h"
#include "arm32.h"
#include "reg-ccu.h"
#include "debug.h"
#include "board.h"
#include "barrier.h"

image_info_t image;
extern u32	 _start;
extern u32	 __spl_start;
extern u32	 __spl_end;
extern u32	 __spl_size;
extern u32	 __stack_srv_start;
extern u32	 __stack_srv_end;
extern u32	 __stack_ddr_srv_start;
extern u32	 __stack_ddr_srv_end;

static unsigned int faterrs;

extern unsigned long sunxi_dram_init(void);

/* Linux zImage Header */
#define LINUX_ZIMAGE_MAGIC 0x016f2818
typedef struct {
	unsigned int code[9];
	unsigned int magic;
	unsigned int start;
	unsigned int end;
} linux_zimage_header_t;

static int boot_image_setup(unsigned char *addr, unsigned int *entry)
{
	linux_zimage_header_t *zimage_header = (linux_zimage_header_t *)addr;

	if (zimage_header->magic == LINUX_ZIMAGE_MAGIC) {
		*entry = ((unsigned int)addr + zimage_header->start);
		return 0;
	}

	error("unsupported kernel image\r\n");

	return -1;
}

#define CHUNK_SIZE 0x20000

static int fatfs_loadimage(char *filename, BYTE *dest)
{
	FIL		 file;
	UINT	 byte_to_read = CHUNK_SIZE;
	UINT	 byte_read;
	UINT	 total_read = 0;
	FRESULT	 fret;
	int		 ret;

	fret = f_open(&file, filename, FA_OPEN_EXISTING | FA_READ);
	if (fret != FR_OK) {
		error("FATFS: open, filename: [%s]: error %d\r\n", filename, fret);
		ret = -1;
		goto open_fail;
	}

	do {
		byte_read = 0;
		fret	  = f_read(&file, (void *)(dest), byte_to_read, &byte_read);
		dest += byte_to_read;
		total_read += byte_read;
	} while (byte_read >= byte_to_read && fret == FR_OK);

	if (fret != FR_OK) {
		error("FATFS: read: error %d\r\n", fret);
		ret = -1;
		goto read_fail;
	}
	ret = 0;

read_fail:
	fret = f_close(&file);

open_fail:
	return ret;
}

#ifdef RESTORE_FEL_ON_FAIL
static void force_fel(sdmmc_pdata_t *card) 
{
	uint8_t buffer[512];
	memset(&buffer, 0, 512);

	error("FATFS: too many errors, restoring FEL boot.");
	sdmmc_blk_write(card, buffer, 16, 1);

	while(2>1);
}
#endif

static int load_sdcard(image_info_t *image, sdmmc_pdata_t *card, int drvnum)
{
	FATFS	fs;
	FRESULT fret;
	int		ret;
	char *pathbuf = "0:";

	*pathbuf += drvnum;

	/* mount fs */
	fret = f_mount(&fs, pathbuf, 1);
	if (fret != FR_OK) {
		error("FATFS: mount error %d: %d\r\n", faterrs, fret);
#ifdef RESTORE_FEL_ON_FAIL
		if (drvnum == 1 && faterrs++ > RESTORE_FEL_MAX_FAIL)
			force_fel(card);
#endif
		return -1;
	} else {
		debug("FATFS: mount OK\r\n");
	}

	*(image->of_filename) += drvnum;
	info("FATFS: read %s addr=%x\r\n", image->of_filename, (unsigned int)image->of_dest);
	ret = fatfs_loadimage(image->of_filename, image->of_dest);
	if (ret)
		return ret;

	*image->filename += drvnum;
	info("FATFS: read %s addr=%x\r\n", image->filename, (unsigned int)image->dest);
	ret = fatfs_loadimage(image->filename, image->dest);
	if (ret)
		return ret;

	/* umount fs */
	fret = f_mount(0, pathbuf, 0);
	if (fret != FR_OK) {
		error("FATFS: unmount error %d\r\n", fret);
		return -1;
	} else {
		debug("FATFS: unmount OK\r\n");
	}

	return 0;
}


static int 
smhc_load(sdhci_t *sdhci, sdmmc_pdata_t *card, image_info_t *image, int drvnum) {

        if (sunxi_sdhci_init(sdhci) != 0) {
                fatal("SMHC: %s controller init failed\r\n", sdhci->name);
        } else {
                info("SMHC: %s controller v%x initialized\r\n", sdhci->name, (unsigned int) sdhci->reg->vers);
        }
        if (sdmmc_init(card, sdhci) != 0) {
                info("SMHC: init failed\r\n");
		return 0;
        }

	*(image->filename) = '0';
	*(image->of_filename) = '0';

        if (load_sdcard(image, card, drvnum) != 0) {
                return 0;
        } else {
                return 1;
        }
}

int main(void)
{
	board_init();
	sunxi_clk_init();

	message("\r\n");
	info("AWBoot r%" PRIu32 " starting...\r\n", (u32)BUILD_REVISION);

	sunxi_dram_init();

	unsigned int entry_point = 0;
	void (*kernel_entry)(int zero, int arch, unsigned int params);

#ifdef CONFIG_ENABLE_CPU_FREQ_DUMP
	sunxi_clk_dump();
#endif

	memset(&image, 0, sizeof(image_info_t));

	image.of_dest = (u8 *)CONFIG_DTB_LOAD_ADDR;
	image.dest	  = (u8 *)CONFIG_KERNEL_LOAD_ADDR;

	strcpy(image.filename, CONFIG_KERNEL_FILENAME);
	strcpy(image.of_filename, CONFIG_DTB_FILENAME);

	while(1) {
		if (smhc_load(&sdhci0, &card0, &image, 0))
			goto _boot;

		if (smhc_load(&sdhci1, &card1, &image, 1))
			goto _boot;
	}


_boot:
	if (boot_image_setup((unsigned char *)image.dest, &entry_point)) {
		fatal("boot setup failed\r\n");
	}

	info("booting linux...\r\n");

	arm32_mmu_disable();
	arm32_dcache_disable();
	arm32_icache_disable();
	arm32_interrupt_disable();

	kernel_entry = (void (*)(int, int, unsigned int))entry_point;
	kernel_entry(0, ~0, (unsigned int)image.of_dest);

	return 0;
}
