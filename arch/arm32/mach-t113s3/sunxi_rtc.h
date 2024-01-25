#ifndef __RTC_H__
#define __RTC_H__

#define SUNXI_RTC_BASE 0x07090000

#define SUNXI_RTC_KEY  0x16aa0000

#define LOSC_CTRL_REG_OFFSET         0x000
#define LOSC_AUTO_SWT_STA_REG_OFFSET 0x004

#define LOSC_SRC_SEL 	         0
#define RTC_SRC_SEL  	         1
#define EXT_LOSC_EN	         4
#define LOSC_AUTO_SWT_32K_SEL_EN 14
#define LOSC_AUTO_SWT_FUNCTION   15


#define LOSC_SRC_SEL_STA 0

typedef enum {
	RTC_CLK_INTERNAL,
	RTC_CLK_EXTERNAL
} sunxi_rtc_clk_source_t;


void sunxi_rtc_setup(sunxi_rtc_clk_source_t source);




#endif /* __RTC_H__ */

