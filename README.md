
Modified version of (https://github.com/szemzoa/awboot)  

SPI is generally removed. Added sdhci1 next to sdhci0 for booting from embedded MMC.

(Modified for T113-S3 Only)



# AWBoot

Small linux bootloader for Allwinner T113-S3

## Building

Run `make LOG_LEVEL=40`.  
This will generate the bootloader with a valid EGON header, usable with the xfel tool or BOOTROM  
You can change the log level with the LOG_LEVEL argument. Default is 30 (info).  

## Using

You will need [xfel](https://github.com/xboot/xfel) for uploading the file to memory or SPI flash.  
This it not needed for writing to an SD card.  

### FEL memory boot:
```
xfel write 0x30000 awboot-fel.bin
xfel exec 0x30000
```

### SD Card boot:
- create an MBR or GPT partition table and a FAT32 partition with an offset of 4MB or more using fdisk.  
```
sudo fdisk /dev/(your sd device)
```
write awboot-boot.bin to sdcard with an offset of:  
- MBR: 8KB (sector 16)
- GPT: 128KB (sector 256)
```
sudo dd if=awboot-boot-sd.bin of=/dev/(your sd device) bs=1024 seek=8
```
- compile (if needed) and copy your `.dtb` file to the FAT partition.
- copy zImage to the FAT partition.

### Linux kernel:
WIP kernel from here: https://github.com/smaeul/linux/tree/d1/all
