#include <stdio.h>
#include <stdarg.h>
#include "io.h"
#include "sunxi_rtc.h"
#include "debug.h"


void sunxi_rtc_setup(sunxi_rtc_clk_source_t source) {

	uint32_t val =
		read32(SUNXI_RTC_BASE + LOSC_CTRL_REG_OFFSET);



	val |= SUNXI_RTC_KEY;
	info("READ VALUE: %x\n", val);

	write32(SUNXI_RTC_BASE + LOSC_CTRL_REG_OFFSET, val);

	val |= (1 << LOSC_SRC_SEL); //Select external crystal as source
	val |= (1 << EXT_LOSC_EN); //enable Crystal
	//val |= (1 << LOSC_AUTO_SWT_FUNCTION); //disable autoswitch
	//val ^= (1 << LOSC_AUTO_SWT_32K_SEL_EN); //disable change to RC

	info("WRITE VALUE: %x\n", val);
	write32(SUNXI_RTC_BASE + LOSC_CTRL_REG_OFFSET, val);

	udelay(500);

	val = read32(SUNXI_RTC_BASE + LOSC_AUTO_SWT_STA_REG_OFFSET);
	while(!(val & (1 << LOSC_SRC_SEL_STA)))
		val = read32(SUNXI_RTC_BASE + LOSC_AUTO_SWT_STA_REG_OFFSET);

	info("External oscillator enabled.\n");

}
