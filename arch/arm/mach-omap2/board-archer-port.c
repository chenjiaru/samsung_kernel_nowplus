/*
 * Copyright (C) 2009 Texas Instruments Inc.
 * Mikkel Christensen <mlc@ti.com>
 *
 * Modified from mach-omap2/board-archer.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/input/matrix_keypad.h>
#include <asm/mach-types.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <asm/mach/arch.h>
#include <linux/bootmem.h>
#include <linux/spi/spi.h>

#include <linux/i2c/twl.h>
#include <linux/interrupt.h>
#include <linux/regulator/machine.h>
#include <linux/switch.h>    
#include <linux/io.h>
#include <linux/gpio.h>
#include <plat/common.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>
#include <plat/board.h>
#include <linux/spi/spi.h>
#include <mach/board-zoom.h>

#include <plat/mcspi.h>
#include <plat/gpio.h>
#include <plat/board.h>
#include <plat/usb.h>
#include <plat/common.h>
#include <plat/dma.h>
#include <plat/gpmc.h>
#include <plat/display.h>
#include <plat/usb.h>
#include <plat/clock.h>
#include <plat/display.h>

#include <asm/io.h>
#include <asm/delay.h>
#include <plat/control.h>
#include <plat/gpmc-smc91x.h>
#include <mach/board-archer.h>
#include <mach/archer.h>
#include <plat/omap-pm.h>
#include <plat/mux_archer_rev_r03.h>
#include <linux/interrupt.h>
#include <plat/control.h>
#include <plat/clock.h>
#include <asm/setup.h>
#include <linux/leds.h>
#include <plat/prcm.h>
#include "cm.h"



#include "mux.h"
#include "sdram-micron-mt46h32m32lf-6.h"
#include "sdram-qimonda-hyb18m512160af-6.h"
#include <linux/ctype.h>
#include "omap3-opp.h"
#include "pm.h"
#include "mmc-twl4030.h"
extern int set_wakeup_gpio( void );
extern int omap_gpio_out_init( void );

#ifdef CONFIG_PM
#include <plat/vrfb.h>
#include <media/videobuf-dma-sg.h>
#include <media/v4l2-device.h>
#include <../../../drivers/media/video/omap/omap_voutdef.h>
#endif

#define ZEUS_CAM
#ifdef ZEUS_CAM
/* include files for cam pmic (power) and cam sensor */
#if (CONFIG_ARCHER_REV == ARCHER_REV00)
#include "../../../drivers/media/video/cam_pmic.h"
#include "../../../drivers/media/video/m4mo.h"
struct m4mo_platform_data zeus_m4mo_platform_data;
#else
#include "../../../drivers/media/video/cam_pmic.h"
#include "../../../drivers/media/video/ce147.h"
#include "../../../drivers/media/video/s5ka3dfx.h"
struct ce147_platform_data zeus_ce147_platform_data;
struct s5ka3dfx_platform_data zeus_s5ka3dfx_platform_data;
#endif
#endif
#if defined(CONFIG_ARCHER_TARGET_SK)
/* make easy to register param to sysfs */
#define REGISTER_PARAM(idx,name)     \
static ssize_t name##_show(struct device *dev, struct device_attribute *attr, char *buf)\
{\
	return get_integer_param(idx, buf);\
}\
static ssize_t name##_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)\
{\
	return set_integer_param(idx, buf, size);\
}\
static DEVICE_ATTR(name, S_IRUGO | S_IWUGO | S_IRUSR | S_IWUSR, name##_show, name##_store)

static struct device *param_dev;

struct class *sec_class;
EXPORT_SYMBOL(sec_class);

struct timezone sec_sys_tz;
EXPORT_SYMBOL(sec_sys_tz);

void (*sec_set_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_set_param_value);

void (*sec_get_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_get_param_value);

/* begins - andre.b.kim : added sec_setprio as module { */
struct class *sec_setprio_class;
EXPORT_SYMBOL(sec_setprio_class);

static struct device *sec_set_prio;

void (*sec_setprio_set_value)(const char *buf);
EXPORT_SYMBOL(sec_setprio_set_value);

void (*sec_setprio_get_value)(void);
EXPORT_SYMBOL(sec_setprio_get_value);
/* } ends - andre.b.kim : added sec_setprio as module */
#endif
u32 hw_revision;
EXPORT_SYMBOL(hw_revision);
#ifdef CONFIG_VIDEO_LV8093
#include <media/lv8093.h>
//extern struct imx046_platform_data zoom2_lv8093_platform_data;
#define LV8093_PS_GPIO			7
/* GPIO7 is connected to lens PS pin through inverter */
#define LV8093_PWR_OFF			1
#define LV8093_PWR_ON			(!LV8093_PWR_OFF)
#endif

#ifndef CONFIG_TWL4030_CORE
#error "no power companion board defined!"
#else
#define TWL4030_USING_BROADCAST_MSG 
#endif

#ifdef CONFIG_WL127X_RFKILL
#include <linux/wl127x-rfkill.h>
#endif

extern int always_opp5;
int usbsel = 1;	
EXPORT_SYMBOL(usbsel);
void (*usbsel_notify)(int) = NULL;
EXPORT_SYMBOL(usbsel_notify);

extern unsigned get_last_off_on_transaction_id(struct device *dev);

#ifdef CONFIG_WL127X_RFKILL
static struct wl127x_rfkill_platform_data wl127x_plat_data = {
	.bt_nshutdown_gpio = OMAP_GPIO_BT_NSHUTDOWN , 	/* Bluetooth Enable GPIO */
	.fm_enable_gpio = -1,		/* FM Enable GPIO */
};

static struct platform_device zoom2_wl127x_device = {
	.name           = "wl127x-rfkill",
	.id             = -1,
	.dev.platform_data = &wl127x_plat_data,
};
#endif
static int zeus_twl4030_keymap[] = {
	KEY(0, 0, KEY_LEFT),
	KEY(0, 1, KEY_RIGHT),
	KEY(0, 2, KEY_A),
	KEY(0, 3, KEY_B),
	KEY(0, 4, KEY_C),
	KEY(1, 0, KEY_DOWN),
	KEY(1, 1, KEY_UP),
	KEY(1, 2, KEY_E),
	KEY(1, 3, KEY_F),
	KEY(1, 4, KEY_G),
	KEY(2, 0, KEY_ENTER),
	KEY(2, 1, KEY_I),
	KEY(2, 2, KEY_J),
	KEY(2, 3, KEY_K),
	KEY(2, 4, KEY_3),
	KEY(3, 0, KEY_M),
	KEY(3, 1, KEY_N),
	KEY(3, 2, KEY_O),
	KEY(3, 3, KEY_P),
	KEY(3, 4, KEY_Q),
	KEY(4, 0, KEY_R),
	KEY(4, 1, KEY_4),
	KEY(4, 2, KEY_T),
	KEY(4, 3, KEY_U),
	KEY(4, 4, KEY_D),
	KEY(5, 0, KEY_V),
	KEY(5, 1, KEY_W),
	KEY(5, 2, KEY_L),
	KEY(5, 3, KEY_S),
	KEY(5, 4, KEY_H),
	0
};

static struct matrix_keymap_data archer_map_data = {
        .keymap                 = zeus_twl4030_keymap,
        .keymap_size            = ARRAY_SIZE(zeus_twl4030_keymap),
};

static struct twl4030_keypad_data archer_kp_data = {
        .keymap_data    = &archer_map_data,
        .rows           = 5,
        .cols           = 6,
        .rep            = 1,
};

static struct resource archer_power_key_resource = {
	.start  = 0,
	.end    = 0,
	.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
};
static struct platform_device archer_power_key_device = {
	.name           = "power_key_device",
	.id             = -1,
	.num_resources  = 1,
	.resource       = &archer_power_key_resource,
};

#ifdef CONFIG_INPUT_ZEUS_EAR_KEY
static struct resource archer_ear_key_resource = {
	.start  = 0,
	.end    = 0,
	.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
};
static struct platform_device archer_ear_key_device = {
	.name           = "ear_key_device",
	.id             = -1,
	.num_resources  = 1,
	.resource       = &archer_ear_key_resource,
};
#endif
static struct resource samsung_charger_resources[] = {
	[0] = {
		// USB IRQ
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_SHAREABLE,
	},
	[1] = {
		// TA IRQ
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE | IORESOURCE_IRQ_LOWEDGE,
	},
	[2] = {
		// CHG_ING_N
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE | IORESOURCE_IRQ_LOWEDGE,
	},
	[3] = {
		// CHG_EN
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ,
	},
};
static int samsung_charger_config_data[] = {
	// [ CHECK VF USING ADC ]
	/*   1. ENABLE  (true, flase) */ 
	true,

	/*   2. ADCPORT (ADCPORT NUM) */
	1,

	
	// [ SUPPORT CHG_ING IRQ FOR CHECKING FULL ]
	/*   1. ENABLE  (true, flase) */
#if (CONFIG_ARCHER_REV >= ARCHER_REV13)
	false,
#else
	true,
#endif
};

static int samsung_battery_config_data[] = {
	// [ SUPPORT MONITORING CHARGE CURRENT FOR CHECKING FULL ]
	/*   1. ENABLE  (true, flase) */
#if (CONFIG_ARCHER_REV >= ARCHER_REV13)
	true,
#else
	false,
#endif
	/*   2. ADCPORT (ADCPORT NUM) */
	4,

	
	// [ SUPPORT MONITORING TEMPERATURE OF THE SYSTEM FOR BLOCKING CHARGE ]
	/*   1. ENABLE  (true, flase) */
	true,
	
	/*   2. ADCPORT (ADCPORT NUM) */
	5,
};
static struct platform_device samsung_charger_device = {
	.name           = "secChargerDev",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(samsung_charger_resources), 
	.resource       = samsung_charger_resources,

	.dev = {
		.platform_data = &samsung_charger_config_data,
	}
};

static struct platform_device samsung_battery_device = {
	.name           = "secBattMonitor",
	.id             = -1,
	.num_resources  = 0, 
	.dev = {
		.platform_data = &samsung_battery_config_data,
	}
};

static struct platform_device samsung_virtual_rtc_device = {
	.name           = "virtual_rtc",
	.id             = -1,
};
 

// cdy_100111 add vibrator device 
static struct platform_device samsung_vibrator_device = {
	.name           = "secVibrator",
	.id             = -1,
	.num_resources  = 0, 
};

static struct platform_device samsung_pl_sensor_power_device = {
	.name           = "secPLSensorPower",
	.id             = -1,
	.num_resources  = 0, 
};


// ryun 20100216 for KEY LED driver
#if defined(CONFIG_ARCHER_TARGET_SK)
static struct led_info sec_keyled_list[] = {
    {
        .name = "button-backlight",        
    },
};
static struct led_platform_data sec_keyled_data = {
    .num_leds   = ARRAY_SIZE(sec_keyled_list),
    .leds       = sec_keyled_list,
};
static struct platform_device samsung_led_device = {
	.name           = "secLedDriver",
	.id             = -1,
	.num_resources  = 0, 
	.dev        = {
        .platform_data  = &sec_keyled_data,
    },
};
#endif
//

//ZEUS_LCD
static struct omap2_mcspi_device_config archer_lcd_mcspi_config = {
	.turbo_mode		= 0,
	.single_channel		= 1,  /* 0: slave, 1: master */
};

//klaatu TDMB SPI
#ifdef CONFIG_TDMB
static struct omap2_mcspi_device_config archer_tdmb_mcspi_config = {
	.turbo_mode     =   0,
	.single_channel =   1,
};
#endif 

static struct spi_board_info archer_spi_board_info[] __initdata = {
#if (CONFIG_ARCHER_REV >= ARCHER_REV10)
	[0] = {
		.modalias		= "ams353_disp_spi",
		.bus_num		= 1,
		.chip_select		= 0,
		.max_speed_hz		= 375000,
		.controller_data 	= &archer_lcd_mcspi_config,
	},//AMS353 LCD
#else
	[0] = {
		.modalias		= "tl2796_disp_spi",
		.bus_num		= 1,
		.chip_select		= 0,
		.max_speed_hz		= 375000,
		.controller_data 	= &archer_lcd_mcspi_config,
	},//ZEUS LCD
#endif

#ifdef CONFIG_TDMB
	[1] = {
		.modalias   =   "tdmbspi",
		.bus_num    =   3,
		.chip_select    =   0,
#ifdef CONFIG_TDMB_T3700
		.max_speed_hz   =  6000000,
#endif
#ifdef CONFIG_TDMB_GDM7024
		.max_speed_hz   =  12000000,
#endif
		.controller_data    = &archer_tdmb_mcspi_config,
	},
#endif //CONFIG_TDMB
};
#if (CONFIG_ARCHER_REV >= ARCHER_REV10)
static struct omap_dss_device archer_lcd_device = {
	.name = "lcd",
	.driver_name = "ams353_panel",
	.type = OMAP_DISPLAY_TYPE_DPI,
	.phy.dpi.data_lines = 24,
	.platform_enable = NULL,
	.platform_disable = NULL,
};
#else
static struct omap_dss_device archer_lcd_device = {
	.name = "lcd",
	.driver_name = "tl2796_panel",
	.type = OMAP_DISPLAY_TYPE_DPI,
	.phy.dpi.data_lines = 24,
	.platform_enable = NULL,
	.platform_disable = NULL,
};
#endif
//ZEUS LCD

static struct omap_dss_device *archer_dss_devices[] = {
	&archer_lcd_device,
};


static struct omap_dss_board_info archer_dss_data = {
	.get_last_off_on_transaction_id = get_last_off_on_transaction_id, 
	.num_devices = ARRAY_SIZE(archer_dss_devices),
	.devices = archer_dss_devices,
	.default_device = &archer_lcd_device,
};

static struct platform_device archer_dss_device = {
	.name          = "omapdss",
	.id            = -1,
	.dev            = {
		.platform_data = &archer_dss_data,
	},
};
#ifdef CONFIG_FB_OMAP2
static struct resource archer_vout_resource[3 - CONFIG_FB_OMAP2_NUM_FBS] = {
};
#else
static struct resource archer_vout_resource[2] = {
};
#endif

#ifdef CONFIG_PM
struct vout_platform_data zeus_vout_data = {
        .set_min_bus_tput = omap_pm_set_min_bus_tput,
        .set_max_mpu_wakeup_lat =  omap_pm_set_max_mpu_wakeup_lat,
        .set_min_mpu_freq = omap_pm_set_min_mpu_freq,
};
#endif 

static struct platform_device zeus_vout_device = {
        .name           = "omap_vout",
        .num_resources  = ARRAY_SIZE(archer_vout_resource),
        .resource       = &archer_vout_resource[0],
        .id             = -1,
#ifdef CONFIG_PM
        .dev            = {
                .platform_data = &zeus_vout_data,
        }
#else
        .dev            = {
                .platform_data = NULL,
        }
#endif
};

static struct gpio_switch_platform_data headset_switch_data = {
	.name = "h2w",
	.gpio =  OMAP_GPIO_DET_3_5, /* Omap3430 GPIO_27 For samsung zeus */
};

static struct platform_device headset_switch_device = {
	.name = "switch-gpio",
	.dev = {
		.platform_data = &headset_switch_data,
	}
};

static struct platform_device sec_device_dpram = {
	.name	= "dpram-device",
	.id	= -1,
};

#if defined(CONFIG_ARCHER_TARGET_SK)
/* SIO Switch */
struct platform_device sec_sio_switch = {
	.name	= "switch-sio",
	.id	= -1,
};
#endif

#define OMAP_GPIO_SYS_DRM_MSECURE 22
static int __init msecure_init(void)
{
        int ret = 0;

#ifdef CONFIG_RTC_DRV_TWL4030
        /* 3430ES2.0 doesn't have msecure/gpio-22 line connected to T2 */
 if (omap_type() == OMAP2_DEVICE_TYPE_GP){
                void __iomem *msecure_pad_config_reg = omap_ctrl_base_get() + 0xA3C;
                int mux_mask = 0x04;
                u16 tmp;
#if 1
                ret = gpio_request(OMAP_GPIO_SYS_DRM_MSECURE, "msecure");
                if (ret < 0) {
                        printk(KERN_ERR "msecure_init: can't"
                                "reserve GPIO:%d !\n", OMAP_GPIO_SYS_DRM_MSECURE);
                        goto out;
                }
                /*
 *  *                  * TWL4030 will be in secure mode if msecure line from OMAP
 *   *                                   * is low. Make msecure line high in order to change the
 *    *                                                    * TWL4030 RTC time and calender registers.
 *     *                                                                     */
                tmp = __raw_readw(msecure_pad_config_reg);
                tmp &= 0xF8;    /* To enable mux mode 03/04 = GPIO_RTC */
                tmp |= mux_mask;/* To enable mux mode 03/04 = GPIO_RTC */
                __raw_writew(tmp, msecure_pad_config_reg);

                gpio_direction_output(OMAP_GPIO_SYS_DRM_MSECURE, 1);
#endif
        }
out:
#endif
        return ret;
}

static struct platform_device *archer_devices[] __initdata = {
	&archer_dss_device,

//	&zeus_vout_device,   //commented to remove double registration of omap_vout
	&headset_switch_device,			
#ifdef CONFIG_WL127X_RFKILL 
	&zoom2_wl127x_device,
#endif
	&archer_power_key_device,
#ifdef CONFIG_INPUT_ZEUS_EAR_KEY
	&archer_ear_key_device,
#endif
	&samsung_battery_device,
	&samsung_charger_device,
	&samsung_vibrator_device,   // cdy_100111 add vibrator device
	&sec_device_dpram,
	&samsung_pl_sensor_power_device,
#if defined(CONFIG_ARCHER_TARGET_SK)
	&samsung_led_device,
	&sec_sio_switch,
#endif
#ifdef CONFIG_RTC_DRV_VIRTUAL
	&samsung_virtual_rtc_device,
#endif

};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = { // WLAN, Alex
        OMAP3_MUX(SDRC_D0,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D1,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D2,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D3,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D4,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D5,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D6,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D7,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D8,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D9,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D10,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D11,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D12,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D13,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D14,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D15,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D16,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D17,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D18,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D19,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D20,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D21,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D22,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D23,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D24,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D25,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D26,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D27,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D28,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D29,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D30,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_D31,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_CKE0,            OMAP34XX_MUX_MODE0),
        OMAP3_MUX(SDRC_CKE1,            OMAP34XX_MUX_MODE0),
        OMAP3_MUX(SDRC_CLK,             OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_DQS0,            OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_DQS1,            OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_DQS2,            OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDRC_DQS3,            OMAP34XX_PIN_INPUT),

        OMAP3_MUX(GPMC_A1,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A2,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A3,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A4,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A5,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A6,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A7,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A8,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A9,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_A10,             OMAP34XX_MUX_MODE7),

        OMAP3_MUX(GPMC_D0,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D1,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D2,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D3,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D4,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D5,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D6,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D7,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D8,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D9,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D10,             OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D11,             OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D12,             OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D13,             OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D14,             OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_D15,             OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_NCS0,            OMAP34XX_MUX_MODE0),
        OMAP3_MUX(GPMC_NCS1,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_NCS2,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_NCS3,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_NCS4,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_NCS5,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_NCS6,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP3_WAKEUP_EN),
        OMAP3_MUX(GPMC_NCS7,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OUTPUT),
        OMAP3_MUX(GPMC_CLK,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_NADV_ALE,        OMAP34XX_MUX_MODE0),
        OMAP3_MUX(GPMC_NOE,                     OMAP34XX_MUX_MODE0),
        OMAP3_MUX(GPMC_NWE,                     OMAP34XX_MUX_MODE0),

        OMAP3_MUX(GPMC_NBE0_CLE,        OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OUTPUT | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(GPMC_NBE1,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OUTPUT | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_NWP,                     OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_WAIT0,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(GPMC_WAIT1,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(GPMC_WAIT2,           OMAP34XX_MUX_MODE7),
        OMAP3_MUX(GPMC_WAIT3,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),

        OMAP3_MUX(DSS_PCLK,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_HSYNC,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_VSYNC,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_ACBIAS,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA0,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA1,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA2,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA3,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA4,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA5,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA6,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA7,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA8,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA9,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA10,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA11,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA12,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA13,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA14,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA15,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA16,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA17,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA18,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA19,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA20,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA21,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(DSS_DATA22,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),
         OMAP3_MUX(DSS_DATA23,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_OUTPUT_LOW),

        OMAP3_MUX(CAM_HS,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_VS,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_XCLKA,            OMAP34XX_MUX_MODE0),
        OMAP3_MUX(CAM_PCLK,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_FLD,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D0,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(CAM_D1,                       OMAP34XX_MUX_MODE7),
        OMAP3_MUX(CAM_D2,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D3,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D4,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D5,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D6,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D7,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D8,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D9,                       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D10,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_D11,                      OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CAM_XCLKB,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(CAM_WEN,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(CAM_STROBE,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),

        OMAP3_MUX(CSI2_DX0,                     OMAP34XX_MUX_MODE7),
#ifdef CONFIG_TDMB
        OMAP3_MUX(CSI2_DY0,                     OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
#else
        OMAP3_MUX(CSI2_DY0,                     OMAP34XX_MUX_MODE7),
#endif
        OMAP3_MUX(CSI2_DX1,                     OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(CSI2_DY1,                     OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),

        OMAP3_MUX(MCBSP2_FSX,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP2_CLKX,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP2_DR,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP2_DX,            OMAP34XX_MUX_MODE0),
         OMAP3_MUX(SDMMC1_CLK,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(SDMMC1_CMD,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC1_DAT0,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC1_DAT1,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC1_DAT2,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC1_DAT3,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC1_DAT4,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SDMMC1_DAT5,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SDMMC1_DAT6,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SDMMC1_DAT7,          OMAP34XX_MUX_MODE7),

#ifdef CONFIG_TDMB
        OMAP3_MUX(SDMMC2_CLK,           OMAP34XX_MUX_MODE1 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(SDMMC2_CMD,           OMAP34XX_MUX_MODE1 | OMAP34XX_PIN_OUTPUT),
        OMAP3_MUX(SDMMC2_DAT0,          OMAP34XX_MUX_MODE1 | OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SDMMC2_DAT1,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SDMMC2_DAT2,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SDMMC2_DAT3,          OMAP34XX_MUX_MODE1 | OMAP34XX_PIN_OUTPUT),
        OMAP3_MUX(SDMMC2_DAT4,          OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OUTPUT),
#else
        OMAP3_MUX(SDMMC2_CLK,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(SDMMC2_CMD,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC2_DAT0,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC2_DAT1,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC2_DAT2,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC2_DAT3,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SDMMC2_DAT4,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
#endif

        OMAP3_MUX(SDMMC2_DAT5,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SDMMC2_DAT6,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SDMMC2_DAT7,          OMAP34XX_MUX_MODE7),

        OMAP3_MUX(MCBSP3_DX,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP3_DR,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OUTPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP3_CLKX,          OMAP34XX_MUX_MODE7),
        OMAP3_MUX(MCBSP3_FSX,           OMAP34XX_MUX_MODE7),

        OMAP3_MUX(UART2_CTS,            OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT),
        OMAP3_MUX(UART2_RTS,            OMAP34XX_MUX_MODE0),
        OMAP3_MUX(UART2_TX,                     OMAP34XX_MUX_MODE0),
        OMAP3_MUX(UART2_RX,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT),

        OMAP3_MUX(UART1_TX,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_OFF_OUTPUT_LOW),
#ifdef FEATURE_SUSPEND_BY_DISABLING_POWER
        OMAP3_MUX(UART1_RTS,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OFF_OUTPUT_LOW),
#else
        OMAP3_MUX(UART1_RTS,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OFF_OUTPUT_HIGH),
#endif
        OMAP3_MUX(UART1_CTS,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(UART1_RX,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_OUTPUT_LOW),

        OMAP3_MUX(MCBSP4_CLKX,          OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP4_DR,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP4_DX,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT),
#ifdef CONFIG_TDMB
        OMAP3_MUX(MCBSP4_FSX,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_OUTPUT),
#else
        OMAP3_MUX(MCBSP4_FSX,           OMAP34XX_MUX_MODE7),
#endif

        OMAP3_MUX(MCBSP1_CLKR,          OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT),
        OMAP3_MUX(MCBSP1_FSR,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP1_DX,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT),
        OMAP3_MUX(MCBSP1_DR,            OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT),
        OMAP3_MUX(MCBSP_CLKS,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCBSP1_FSX,           OMAP34XX_MUX_MODE7),
        OMAP3_MUX(MCBSP1_CLKX,          OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),

        OMAP3_MUX(UART3_CTS_RCTX,       OMAP34XX_MUX_MODE7),
        OMAP3_MUX(UART3_RTS_SD,         OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(UART3_RX_IRRX,        OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_WAKEUPENABLE),
        OMAP3_MUX(UART3_TX_IRTX,        OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_OFF_OUTPUT_LOW),

        // HSUSB SIGNALS - skip
        OMAP3_MUX(HSUSB0_CLK,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_STP,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_OUTPUT_HIGH),
        OMAP3_MUX(HSUSB0_DIR,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_NXT,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA0,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA1,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA2,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA3,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA4,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA5,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA6,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(HSUSB0_DATA7,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),

        OMAP3_MUX(I2C1_SCL,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(I2C1_SDA,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(I2C2_SCL,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(I2C2_SDA,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP),
#ifndef FEATURE_SUSPEND_BY_DISABLING_POWER
        OMAP3_MUX(I2C3_SCL,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_NONE),
        OMAP3_MUX(I2C3_SDA,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_NONE),
#else
        OMAP3_MUX(I2C3_SCL,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(I2C3_SDA,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_OUTPUT_LOW),
#endif

        OMAP3_MUX(HDQ_SIO,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),

        OMAP3_MUX(MCSPI1_CLK,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI1_SIMO,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI1_SOMI,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI1_CS0,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI1_CS1,           OMAP34XX_MUX_MODE3 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(MCSPI1_CS2,           OMAP34XX_MUX_MODE3 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(MCSPI1_CS3,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI2_CLK,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(MCSPI2_SIMO,          OMAP34XX_MUX_MODE1 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI2_SOMI,          OMAP34XX_MUX_MODE4| OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI2_CS0,           OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(MCSPI2_CS1,           OMAP34XX_MUX_MODE7),

        OMAP3_MUX(SYS_NIRQ,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP3_WAKEUP_EN),
#ifndef FEATURE_SUSPEND_BY_DISABLING_POWER
        OMAP3_MUX(SYS_CLKOUT2,          OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_NONE),
#else
        OMAP3_MUX(SYS_CLKOUT2,          OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_OUTPUT_LOW),
#endif
        OMAP3_MUX(SAD2D_MCAD0,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD1,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD2,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD3,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD4,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD5,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD6,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD7,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD8,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD9,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD10,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD11,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD12,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD13,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD14,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD15,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD16,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD17,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD18,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD19,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD20,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD21,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD22,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD23,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD24,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD25,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD26,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD27,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD28,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD29,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD30,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD31,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD32,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD33,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD34,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD35,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MCAD36,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_CLK26MI,              OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(CHASSIS_NRESPWRON,    OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(CHASSIS_NRESWARW,             OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLUP),
        OMAP3_MUX(CHASSIS_NIRQ,                 OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(CHASSIS_FIQ,                  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_OUTPUT_LOW),
        OMAP3_MUX(CHASSIS_ARMIRQ,               OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_IVAIRQ,               OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_DMAREQ0,              OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_DMAREQ1,              OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_DMAREQ2,              OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_DMAREQ3,              OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),

        OMAP3_MUX(CHASSIS_NTRST,                OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_TDI,                  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_TDO,                  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_TMS,                  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_TCK,                  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_RTCK,                 OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_MSTDBY,               OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLUP),
        OMAP3_MUX(CHASSIS_IDLEREQ,              OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(CHASSIS_IDLEACK,              OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLUP),

        OMAP3_MUX(SAD2D_MWRITE,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_SWRITE,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MREAD,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_SREAD,          OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_MBUSFLAG,       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(SAD2D_SBUSFLAG,       OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),

        OMAP3_MUX(I2C4_SCL,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(I2C4_SDA,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP),

        OMAP3_MUX(SYS_32K,                      OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SYS_CLKREQ,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT),
        OMAP3_MUX(SYS_NRESWARM,         OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT),

        OMAP3_MUX(SYS_BOOT0,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SYS_BOOT1,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SYS_BOOT2,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SYS_BOOT3,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SYS_BOOT4,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SYS_BOOT5,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(SYS_BOOT6,            OMAP34XX_MUX_MODE7),

        OMAP3_MUX(SYS_OFF_MODE,         OMAP34XX_MUX_MODE0),
        OMAP3_MUX(SYS_CLKOUT1,          OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP3_WAKEUP_EN),

        OMAP3_MUX(JTAG_NTRST,           OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(JTAG_TCK,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(JTAG_TMS_TMSC,        OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(JTAG_TDI,                     OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(JTAG_EMU0,            OMAP34XX_MUX_MODE7),
        OMAP3_MUX(JTAG_EMU1,            OMAP34XX_MUX_MODE7),

        OMAP3_MUX(ETK_CLK,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP3_WAKEUP_EN),
        OMAP3_MUX(ETK_CTL,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(ETK_D0,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(ETK_D1,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP3_WAKEUP_EN),
        OMAP3_MUX(ETK_D2,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(ETK_D3,                       OMAP34XX_MUX_MODE2 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(ETK_D4,                       OMAP34XX_MUX_MODE2 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(ETK_D5,                       OMAP34XX_MUX_MODE2 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(ETK_D6,                       OMAP34XX_MUX_MODE2 | OMAP34XX_PIN_INPUT_PULLUP),
//      OMAP3_MUX(ETK_D7,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP34XX_PIN_OFF_INPUT_PULLDOWN),
        OMAP3_MUX(ETK_D7,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN),
        OMAP3_MUX(ETK_D8,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP),
        OMAP3_MUX(ETK_D9,                       OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT|OMAP3_WAKEUP_EN),
        OMAP3_MUX(ETK_D10,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLDOWN | OMAP3_WAKEUP_EN),
        OMAP3_MUX(ETK_D11,                      OMAP34XX_MUX_MODE7),
        OMAP3_MUX(ETK_D12,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT | OMAP3_WAKEUP_EN),
        OMAP3_MUX(ETK_D13,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP3_WAKEUP_EN),
        OMAP3_MUX(ETK_D14,                      OMAP34XX_MUX_MODE4 | OMAP34XX_PIN_INPUT_PULLUP | OMAP3_WAKEUP_EN),
        OMAP3_MUX(ETK_D15,                      OMAP34XX_MUX_MODE7),

        OMAP3_MUX(SAD2D_SWAKEUP,        OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT),
        { .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux       NULL
#endif

static inline void __init archer_init_power_key(void)
{
	archer_power_key_resource.start = gpio_to_irq(OMAP_GPIO_KEY_PWRON);
	if (gpio_request(OMAP_GPIO_KEY_PWRON, "power_key_irq") < 0 ) {
		printk(KERN_ERR"\n FAILED TO REQUEST GPIO %d for POWER KEY IRQ \n",OMAP_GPIO_KEY_PWRON);
		return;
	}
	gpio_direction_input(OMAP_GPIO_KEY_PWRON);
}

#ifdef CONFIG_INPUT_ZEUS_EAR_KEY
static inline void __init archer_init_ear_key(void)
{
	archer_ear_key_resource.start = gpio_to_irq(OMAP_GPIO_EAR_KEY);
	if (gpio_request(OMAP_GPIO_EAR_KEY, "ear_key_irq") < 0 ) {
		printk(KERN_ERR"\n FAILED TO REQUEST GPIO %d for POWER KEY IRQ \n",OMAP_GPIO_EAR_KEY);
		return;
	}
	gpio_direction_input(OMAP_GPIO_EAR_KEY);

}
#endif

static inline void __init archer_init_battery(void)
{
#if (CONFIG_ARCHER_REV >= ARCHER_REV13)	
	samsung_charger_resources[0].start = 0; // gpio_to_irq(OMAP_GPIO_USBSW_NINT);;    
	if (gpio_request(OMAP_GPIO_TA_NCONNECTED, "ta_nconnected irq") < 0) {
		printk(KERN_ERR "Failed to request GPIO%d for ta_nconnected IRQ\n",
		       OMAP_GPIO_TA_NCONNECTED);
		samsung_charger_resources[1].start = -1;
	}
	else {
		samsung_charger_resources[1].start = gpio_to_irq(OMAP_GPIO_TA_NCONNECTED);
		omap_set_gpio_debounce_time( OMAP_GPIO_TA_NCONNECTED, 3 );
		omap_set_gpio_debounce( OMAP_GPIO_TA_NCONNECTED, true );
	}
#else

#if defined(CONFIG_FSA9480_MICROUSB)
	samsung_charger_resources[0].start = gpio_to_irq(OMAP_GPIO_USBSW_NINT);
	samsung_charger_resources[1].start = gpio_to_irq(57); // NC
#elif defined(CONFIG_MICROUSBIC_INTR)
	samsung_charger_resources[0].start = IH_USBIC_BASE;    
	samsung_charger_resources[1].start = IH_USBIC_BASE + 1;
#endif
	
#endif
	if (gpio_request(OMAP_GPIO_CHG_ING_N, "charge full irq") < 0) {
		printk(KERN_ERR "Failed to request GPIO%d for charge full IRQ\n",
		       OMAP_GPIO_CHG_ING_N);
		samsung_charger_resources[2].start = -1;
	}
	else {
		samsung_charger_resources[2].start = gpio_to_irq(OMAP_GPIO_CHG_ING_N);
		omap_set_gpio_debounce_time( OMAP_GPIO_CHG_ING_N, 3 );
		omap_set_gpio_debounce( OMAP_GPIO_CHG_ING_N, true );
	}

	if (gpio_request(OMAP_GPIO_CHG_EN, "Charge enable gpio") < 0) {
		printk(KERN_ERR "Failed to request GPIO%d for charge enable gpio\n",
		       OMAP_GPIO_CHG_EN);
		samsung_charger_resources[3].start = -1;
 	}
	else {
		samsung_charger_resources[3].start = gpio_to_irq(OMAP_GPIO_CHG_EN);
	}
	
}
static void __init omap_archer_init_irq(void)
{
	omap_init_irq();
        omap2_init_common_hw(hyb18m512160af6_sdrc_params,
                                 NULL, omap3_mpu_rate_table,
                                 omap3_dsp_rate_table, omap3_l3_rate_table);
	//omap_gpio_init();  //Crashing in kernel-2.6.32

	//zeus_init_smc91x();

	archer_init_power_key();
#ifdef CONFIG_INPUT_ZEUS_EAR_KEY
	archer_init_ear_key();
#endif
	archer_init_battery();

}

static struct regulator_consumer_supply archer_vdda_dac_supply = {
	.supply		= "vdda_dac",
	.dev		= &archer_dss_device.dev,
};



/* REVISIT: These audio entries can be removed once MFD code is merged */


static struct regulator_consumer_supply archer_vsim_supply = {
	.supply		= "vmmc_aux",
};


static struct regulator_consumer_supply archer_vmmc1_supply = {
	.supply		= "vmmc",
};

static struct regulator_consumer_supply archer_vmmc2_supply = {
	.supply		= "vmmc",
};
static struct regulator_consumer_supply archer_vaux1_supply = {
        .supply         = "vaux1",
};

static struct regulator_consumer_supply archer_vaux3_supply = {
        .supply         = "vaux3",
};

static struct regulator_consumer_supply archer_vaux4_supply = {
        .supply         = "vaux4",
};

static struct regulator_consumer_supply archer_vpll2_supply = {
        .supply         = "vpll2",
};

struct regulator_init_data archer_vdac = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &archer_vdda_dac_supply,
};

/* VMMC1 for OMAP VDD_MMC1 (i/o) and MMC1 card */
static struct regulator_init_data archer_vmmc1 = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 3150000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &archer_vmmc1_supply,
};


/* VMMC2 for MMC2 card */
static struct regulator_init_data archer_vmmc2 = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 1850000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &archer_vmmc2_supply,
};
/* VAUX1 for PL_SENSOR */
static struct regulator_init_data archer_aux1 = {
	.constraints = {
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &archer_vaux1_supply,
};

/* VSIM for OMAP VDD_MMC1A (i/o for DAT4..DAT7) */
static struct regulator_init_data archer_vsim = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 3000000,
		.always_on		= true,            
		.boot_on		= true,				
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &archer_vsim_supply,
};

/* VAUX3 for LCD */
static struct regulator_init_data archer_aux3 = {
        .constraints = {
                .min_uV                 = 1800000,
                .max_uV                 = 1800000,
                .boot_on                = true,
                .valid_modes_mask       = REGULATOR_MODE_NORMAL
                                        | REGULATOR_MODE_STANDBY,
                .valid_ops_mask         = REGULATOR_CHANGE_MODE
                                        | REGULATOR_CHANGE_STATUS,
        },
        .num_consumer_supplies  = 1,
        .consumer_supplies      = &archer_vaux3_supply,
};

/* VAUX4 for LCD */
static struct regulator_init_data archer_aux4 = {
        .constraints = {
                .min_uV                 = 2800000,
                .max_uV                 = 2800000,
                .boot_on                = true,
                .valid_modes_mask       = REGULATOR_MODE_NORMAL
                                        | REGULATOR_MODE_STANDBY,
                .valid_ops_mask         =  REGULATOR_CHANGE_MODE
                                        | REGULATOR_CHANGE_STATUS,
        },
        .num_consumer_supplies  = 1,
        .consumer_supplies      = &archer_vaux4_supply,
};


/* VPLL2 for LCD */
static struct regulator_init_data archer_vpll2 = {
        .constraints = {
                .min_uV                 = 1800000,
                .max_uV                 = 1800000,
                .boot_on                = true,
                .valid_modes_mask       = REGULATOR_MODE_NORMAL
                                        | REGULATOR_MODE_STANDBY,
                .valid_ops_mask         = REGULATOR_CHANGE_MODE
                                        | REGULATOR_CHANGE_STATUS,
        },
        .num_consumer_supplies  = 1,
        .consumer_supplies      = &archer_vpll2_supply,
};






//ZEUS_LCD
static struct omap_lcd_config archer_lcd_config __initdata = {
	.ctrl_name      = "internal",
};
//ZEUS_LCD
static struct omap_uart_config archer_uart_config __initdata = {
#ifdef CONFIG_SERIAL_OMAP_CONSOLE
	.enabled_uarts	= ( (1 << 0) | (1 << 1) | (1 << 2) ),
#else
	.enabled_uarts	= ( (1 << 0) | (1 << 1) ),
#endif
};

static struct omap_board_config_kernel archer_config[] __initdata = {
	{ OMAP_TAG_UART,	&archer_uart_config },
	{ OMAP_TAG_LCD,         &archer_lcd_config }, //ZEUS_LCD
};

#if 1 //def _USE_TWL_BCI_MODULE_
static int archer_batt_table[] = {
/* 0 C*/
30800, 29500, 28300, 27100,
26000, 24900, 23900, 22900, 22000, 21100, 20300, 19400, 18700, 17900,
17200, 16500, 15900, 15300, 14700, 14100, 13600, 13100, 12600, 12100,
11600, 11200, 10800, 10400, 10000, 9630,  9280,  8950,  8620,  8310,
8020,  7730,  7460,  7200,  6950,  6710,  6470,  6250,  6040,  5830,
5640,  5450,  5260,  5090,  4920,  4760,  4600,  4450,  4310,  4170,
4040,  3910,  3790,  3670,  3550
};

static struct twl4030_bci_platform_data archer_bci_data = {
	.battery_tmp_tbl	= archer_batt_table,
	.tblsize		= ARRAY_SIZE(archer_batt_table),
};
#endif

static struct twl4030_hsmmc_info mmc[] __initdata = {
	{
		.name		= "external",
		.mmc		= 1,
		.wires		= 4,
		.gpio_wp	= -EINVAL,
		.power_saving	= true,
	},
#if !defined(CONFIG_ARCHER_TARGET_SK)
	{
		.name		= "internal",
		.mmc		= 2,
		.wires		= 8,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.nonremovable	= true,
		.power_saving	= true,
	},
#endif
	{
		.mmc		= 3,
		.wires		= 4,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},	
	{}      /* Terminator */
};



static int archer_twl_gpio_setup(struct device *dev, unsigned gpio, unsigned ngpio)
{
	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
	//mmc[0].gpio_cd = gpio + 0;
	mmc[0].gpio_cd = 23;
	twl4030_mmc_init(mmc);

	/* link regulators to MMC adapters ... we "know" the
	 * regulators will be set up only *after* we return.
	*/
	archer_vmmc1_supply.dev = mmc[0].dev;
	archer_vsim_supply.dev = mmc[0].dev;
	archer_vmmc2_supply.dev = mmc[1].dev;

	return 0;
}
static struct twl4030_gpio_platform_data archer_gpio_data = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.setup		= archer_twl_gpio_setup,
};

static struct twl4030_usb_data archer_usb_data = {
	.usb_mode	= T2_USB_MODE_ULPI,
};
static struct twl4030_madc_platform_data archer_madc_data = {
	.irq_line	= 1,
};
static struct twl4030_ins __initdata sleep_on_seq[] = {
#ifdef TWL4030_USING_BROADCAST_MSG
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_RC, RES_TYPE_ALL,0x0,RES_STATE_OFF), 2},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_ALL,0x0,RES_STATE_SLEEP), 2},

#else
	/* Turn off HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_OFF), 2},
	/* Turn OFF VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_OFF), 2},
	/* Turn OFF VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_OFF), 2},
	/* Turn OFF VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_OFF), 2},
#endif
};

static struct twl4030_script sleep_on_script __initdata = {
	.script	= sleep_on_seq,
	.size	= ARRAY_SIZE(sleep_on_seq),
	.flags	= TRITON_SLEEP_SCRIPT,
};

static struct twl4030_ins wakeup_p12_seq[] __initdata = {
#ifdef TWL4030_USING_BROADCAST_MSG
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_RES, RES_TYPE_ALL,0x2,RES_STATE_ACTIVE), 2},
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_ALL,0x0,RES_STATE_ACTIVE), 2},
#else
	/* Turn on HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},
	/* Turn ON VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_ACTIVE), 2},
	/* Turn ON VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_ACTIVE), 2},
	/* Turn ON VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_ACTIVE), 2},
#endif
};

static struct twl4030_script wakeup_p12_script __initdata = {
	.script = wakeup_p12_seq,
	.size   = ARRAY_SIZE(wakeup_p12_seq),
	.flags  = TWL4030_WAKEUP12_SCRIPT,
};
static struct twl4030_ins wakeup_p3_seq[] __initdata = {
#ifdef TWL4030_USING_BROADCAST_MSG
	{MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_RC, RES_TYPE_ALL,0x0,RES_STATE_ACTIVE), 2},
#else
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},
#endif
};

static struct twl4030_script wakeup_p3_script __initdata = {
	.script = wakeup_p3_seq,
	.size   = ARRAY_SIZE(wakeup_p3_seq),
	.flags  = TWL4030_WAKEUP3_SCRIPT,
};

static struct twl4030_ins wrst_seq[] __initdata = {
/*
 * Reset twl4030.
 * Reset VDD1 regulator.
 * Reset VDD2 regulator.
 * Reset VPLL1 regulator.
 * Enable sysclk output.
 * Reenable twl4030.
 */
 #ifdef NOT_SUPPORT_HW_RESET_DURING_SLEEP
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_OFF), 2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_WRST), 15},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_WRST), 15},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_WRST), 0x60},
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_ACTIVE), 2},
#else
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_OFF), 2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_Main_Ref, RES_STATE_WRST), 2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VIO, RES_STATE_ACTIVE), 15},            // VIO active
	{MSG_SINGULAR(DEV_GRP_P3, RES_CLKEN, RES_STATE_ACTIVE), 2},          // CLKEN active
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_ACTIVE), 2},           // VPLL1 active
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_ACTIVE), 15},          // VDD2 active
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_ACTIVE), 25},          // VDD1 active
// {MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_ALL, RES_TYPE_ALL, RES_STATE_WRST), 75},            // All resources reset
	{MSG_BROADCAST(DEV_GRP_P1, RES_GRP_RC, RES_TYPE_ALL,RES_TYPE_ALL,RES_STATE_ACTIVE), 2},                    // All RC resources active
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_ACTIVE), 2},
#endif
};
static struct twl4030_script wrst_script __initdata = {
	.script = wrst_seq,
	.size   = ARRAY_SIZE(wrst_seq),
	.flags  = TWL4030_WRST_SCRIPT,
};

static struct twl4030_script *twl4030_scripts[] __initdata = {
	&sleep_on_script,
	&wakeup_p12_script,
	&wakeup_p3_script,
	&wrst_script,
};

static struct twl4030_resconfig twl4030_rconfig[] = {
	{ .resource = RES_HFCLKOUT, .devgroup = DEV_GRP_P3, .type = -1, .type2 = -1 },
#ifdef TWL4030_USING_BROADCAST_MSG
	{ .resource = RES_CLKEN, .devgroup = DEV_GRP_P3, .type = -1, .type2 = -1 },
	{ .resource = RES_VAUX1, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VAUX2, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
#ifndef CONFIG_FB_OMAP_BOOTLOADER_INIT
	{ .resource = RES_VAUX3, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VAUX4, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
#endif
	{ .resource = RES_VMMC1, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VMMC2, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VPLL2, .devgroup = DEV_GRP_P1, .type = -1, .type2 = -1 },
	{ .resource = RES_VSIM, .devgroup = DEV_GRP_P1, .type = -1, .type2 = -1 },
	{ .resource = RES_VDAC, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VUSB_1V5, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VUSB_1V8, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VUSB_3V1, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VINTANA1, .devgroup = DEV_GRP_P1, .type = -1, .type2 = -1 },
	{ .resource = RES_VINTANA2, .devgroup = DEV_GRP_NULL, .type = -1, .type2 = -1 },
	{ .resource = RES_VINTDIG, .devgroup = DEV_GRP_P1, .type = -1, .type2 = -1 },
#endif
	{ .resource = RES_VDD1, .devgroup = DEV_GRP_P1, .type = -1, .type2 = -1 },
	{ .resource = RES_VDD2, .devgroup = DEV_GRP_P1, .type = -1, .type2 = -1 },
	{ 0, 0},
};

static struct twl4030_power_data archer_t2scripts_data __initdata = {
	.scripts	= twl4030_scripts,
	.num		= ARRAY_SIZE(twl4030_scripts),
	.resource_config = twl4030_rconfig,
};

static struct twl4030_codec_audio_data archer_audio_data = {
	.audio_mclk = 26000000,
};

static struct twl4030_codec_data archer_codec_data = {
	.audio_mclk = 26000000,
	.audio = &archer_audio_data,
};

static struct twl4030_platform_data archer_twldata = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	/* platform_data for children goes here */
	.bci		= &archer_bci_data,
	.madc		= &archer_madc_data,
	.usb		= &archer_usb_data,
	.gpio		= &archer_gpio_data,
	.keypad		= &archer_kp_data,
	.codec		= &archer_codec_data,
	.power		= &archer_t2scripts_data,
	.vmmc1		= &archer_vmmc1,
	.vmmc2		= &archer_vmmc2,
	.vsim		= &archer_vsim,
	.vdac		= &archer_vdac,
	.vaux1		= &archer_aux1,
        .vaux3          = &archer_aux3,
        .vaux4          = &archer_aux4,
        .vpll2          = &archer_vpll2,
};

#if  0 //(CONFIG_ARCHER_REV < ARCHER_REV10)
static void synaptics_dev_init(void)
{
	if (gpio_request(OMAP_GPIO_TOUCH_IRQ, "touch") < 0) {
		printk(KERN_ERR "can't get synaptics pen down GPIO\n");
		return;
	}
	gpio_direction_input(OMAP_GPIO_TOUCH_IRQ);
	omap_set_gpio_debounce(OMAP_GPIO_TOUCH_IRQ, 0);  //  added this to remove the debouncing feature
}
static int synaptics_power(int power_state)
{
	int ret  = 0;
	if(power_state)
	{
		if (ret != twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,0x09, TWL4030_VAUX2_DEDICATED))
			return -EIO;
		if (ret != twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,0x20, TWL4030_VAUX2_DEV_GRP))
			return -EIO;
	}
	else
	{
		if (ret != twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,0x00, TWL4030_VAUX2_DEV_GRP))
			return -EIO;
	}

// WL127X Power on by eugene : It affect lower throughput
	if (ret != twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,0xE0, TWL4030_VSIM_DEV_GRP)) {
		return -EIO;
	}
	printk(KERN_ERR "twl4030_i2c_write_u8 called\n");

	return 0;
}
static struct synaptics_i2c_rmi_platform_data synaptics_platform_data[] = {
	{
		.version	= 0x0,
		.power		= &synaptics_power,
		.flags		= SYNAPTICS_SWAP_XY,
	}
};
#endif
#ifdef CONFIG_MICROUSBIC_INTR
static void microusbic_dev_init(void)        
{
	if (omap_request_gpio(OMAP_GPIO_USBSW_NINT) < 0) {
		printk(KERN_ERR " GFree can't get microusb pen down GPIO\n");
		return;
	}
	omap_set_gpio_direction(OMAP_GPIO_USBSW_NINT, 1);   
}
#endif
 struct i2c_board_info __initdata archer_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("twl5030", 0x48),
		.flags		= I2C_CLIENT_WAKE,
		.irq		= INT_34XX_SYS_NIRQ,
		.platform_data	= &archer_twldata,
	},
};

static struct i2c_board_info __initdata archer_i2c_boardinfo1[] = {
#if defined(CONFIG_FSA9480_MICROUSB)
	{
		I2C_BOARD_INFO("fsa9480", 0x25),
		.flags = I2C_CLIENT_WAKE,
		.irq = OMAP_GPIO_IRQ(OMAP_GPIO_USBSW_NINT),
	},
#elif defined(CONFIG_MICROUSBIC_INTR)
	{
		I2C_BOARD_INFO("microusbic", 0x25),
	},
#endif

	{
		I2C_BOARD_INFO("max9877", 0x4d),

	},
#if (CONFIG_ARCHER_REV == ARCHER_REV00)
	{
		I2C_BOARD_INFO(M4MO_DRIVER_NAME, M4MO_I2C_ADDR),
		.platform_data = &zeus_m4mo_platform_data,
	},
#else
	{
		I2C_BOARD_INFO(CE147_DRIVER_NAME, CE147_I2C_ADDR),
		.platform_data = &zeus_ce147_platform_data,
	},
	{
		I2C_BOARD_INFO(S5KA3DFX_DRIVER_NAME, S5KA3DFX_I2C_ADDR),
		.platform_data = &zeus_s5ka3dfx_platform_data,
	},	
#endif	
	{
		I2C_BOARD_INFO("cam_pmic", CAM_PMIC_I2C_ADDR),		
	},
	{
		I2C_BOARD_INFO("secFuelgaugeDev", 0x36),
#if (CONFIG_ARCHER_REV >= ARCHER_REV11)
		.flags = I2C_CLIENT_WAKE,
		.irq = OMAP_GPIO_IRQ(OMAP_GPIO_FUEL_INT_N),
#endif
	},
	{
		I2C_BOARD_INFO("PL_driver", 0x44),
	},
#if (CONFIG_ARCHER_REV >= ARCHER_REV13)
	{
		I2C_BOARD_INFO("kxsd9", 0x18),
	},
#else
	{
		I2C_BOARD_INFO("bma023", 0x38),
	},
#endif
	{
		I2C_BOARD_INFO("ak8973b", 0x1c),
	},


};

#if 0
//#if defined(CONFIG_VIDEO_LV8093) && defined(CONFIG_VIDEO_OMAP3)
	{
		I2C_BOARD_INFO(LV8093_NAME,  LV8093_AF_I2C_ADDR),
		.platform_data = &zoom2_lv8093_platform_data,
	},
#endif


#if (CONFIG_ARCHER_REV < ARCHER_REV10)

static struct i2c_board_info __initdata archer_i2c_boardinfo_3[] = {           //Added for i2c3 register-CY8
        {
                I2C_BOARD_INFO(SYNAPTICS_I2C_RMI_NAME,  SYNAPTICS_I2C_ADDR),
                .irq = 0,
                .platform_data = &synaptics_platform_data,
        },
};
#endif

#if (CONFIG_ARCHER_REV >= ARCHER_REV10)

static struct i2c_board_info __initdata archer_i2c_boardinfo_4[] = {           //Added for i2c3 register-CY8

        {
                I2C_BOARD_INFO("melfas_ts",  0x40),     // 10010(A1)(A0)  A1=PD0, A0=M(0=12bit, 1=8bit)
                .type = "melfas_ts",
        //       .platform_data = &tsc2007_info,
         },
        
};
#endif

static int __init omap_i2c_init(void)
{
/* Disable OMAP 3630 internal pull-ups for I2Ci */
	if (cpu_is_omap3630()) {

		u32 prog_io;

		prog_io = omap_ctrl_readl(OMAP343X_CONTROL_PROG_IO1);
		/* Program (bit 19)=1 to disable internal pull-up on I2C1 */
		prog_io |= OMAP3630_PRG_I2C1_PULLUPRESX;
		/* Program (bit 0)=1 to disable internal pull-up on I2C2 */
		prog_io |= OMAP3630_PRG_I2C2_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP343X_CONTROL_PROG_IO1);

		prog_io = omap_ctrl_readl(OMAP36XX_CONTROL_PROG_IO2);
		/* Program (bit 7)=1 to disable internal pull-up on I2C3 */
		prog_io |= OMAP3630_PRG_I2C3_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP36XX_CONTROL_PROG_IO2);

		prog_io = omap_ctrl_readl(OMAP36XX_CONTROL_PROG_IO_WKUP1);
		/* Program (bit 5)=1 to disable internal pull-up on I2C4(SR) */
		prog_io |= OMAP3630_PRG_SR_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP36XX_CONTROL_PROG_IO_WKUP1);
	}
/*
	omap_register_i2c_bus(1, 2400, zoom_i2c_boardinfo,
			ARRAY_SIZE(zoom_i2c_boardinfo));
	omap_register_i2c_bus(2, 400, NULL, 0);
	omap_register_i2c_bus(3, 400, NULL, 0);
*/
#if 0
	omap_register_i2c_bus(1, 100, archer_i2c_boardinfo,
			ARRAY_SIZE(archer_i2c_boardinfo));
	omap_register_i2c_bus(2, 100, archer_i2c_boardinfo1,
		ARRAY_SIZE(archer_i2c_boardinfo1));
	omap_register_i2c_bus(3, 400,archer_i2c_boardinfo_4,ARRAY_SIZE(archer_i2c_boardinfo_4));
#else
	/* CSR ID:- OMAPS00222372 Changed the order of I2C Bus Registration 
 	*  Previously I2C1 channel 1 was being registered followed by I2C2 but since
 	*  TWL4030-USB module had a dependency on FSA9480 USB Switch device which is
 	*  connected to I2C2 channel, changed the order such that I2C channel 2 will get
 	*  registered first and then followed by I2C1 channel. */
	omap_register_i2c_bus(2, 200, archer_i2c_boardinfo1,
		ARRAY_SIZE(archer_i2c_boardinfo1));
	omap_register_i2c_bus(1, 400, archer_i2c_boardinfo,
			ARRAY_SIZE(archer_i2c_boardinfo));
	omap_register_i2c_bus(3, 100,archer_i2c_boardinfo_4,ARRAY_SIZE(archer_i2c_boardinfo_4));
#endif
	return 0;
}

static void config_wlan_gpio(void)
{
        int ret = 0;

#if 1//TI HS.Yoon 20100827 for enabling WLAN_IRQ wakeup
	omap_writel(omap_readl(0x480025E8)|0x410C0000, 0x480025E8);
	omap_writew(0x10C, 0x48002194);
#endif	
        ret = gpio_request(OMAP_GPIO_WLAN_IRQ, "wifi_irq");
        if (ret < 0) {
                printk(KERN_ERR "%s: can't reserve GPIO: %d\n", __func__,
                        OMAP_GPIO_WLAN_IRQ);
                return;
        }
        ret = gpio_request(OMAP_GPIO_WLAN_EN, "wifi_pmena");
        if (ret < 0) {
                printk(KERN_ERR "%s: can't reserve GPIO: %d\n", __func__,
                        OMAP_GPIO_WLAN_EN);
                gpio_free(OMAP_GPIO_WLAN_EN);
                return;
        }
        gpio_direction_input(OMAP_GPIO_WLAN_IRQ);
        gpio_direction_output(OMAP_GPIO_WLAN_EN, 0);

#ifdef __TRISCOPE__ 
        gpio_set_value(OMAP_GPIO_WLAN_EN, 1);
#endif
}


#if defined (CONFIG_MACH_ARCHER)
static void config_camera_gpio(void)
{
	if (gpio_request(OMAP3430_GPIO_CAMERA_EN,"CAM EN") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d\n", OMAP3430_GPIO_CAMERA_EN);
		return;
	} 

	if (gpio_request(OMAP3430_GPIO_CAMERA_1P2V_EN,"CAMERA 1P2V EN") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d\n", OMAP3430_GPIO_CAMERA_1P2V_EN);
		return;
	}

	if (gpio_request(OMAP3430_GPIO_CAMERA_1P8V_EN,"CAMERA 1P8V EN") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d\n", OMAP3430_GPIO_CAMERA_1P8V_EN);
		return;
	}    

	if (gpio_request(OMAP3430_GPIO_CAMERA_STBY,"CAM STBY") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d\n", OMAP3430_GPIO_CAMERA_STBY);
		return;
	}    

	if (gpio_request(OMAP3430_GPIO_CAMERA_RST,"CAM RST") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d\n", OMAP3430_GPIO_CAMERA_RST);
		return;
	}      

	if (gpio_request(OMAP3430_GPIO_VGA_STBY,"CAM VGA_STBY") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_VGA_STBY);
		return;
	}       

	if (gpio_request(OMAP3430_GPIO_VGA_RST,"CAM VGA RST") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_VGA_RST);
		return;
	}

	if (gpio_request(OMAP3430_GPIO_VGA_SEL,"CAMERA VGA SELECT") != 0) 
	{
		printk(CE147_MOD_NAME "Could not request GPIO %d\n", OMAP3430_GPIO_VGA_SEL);
		return;
	}
}
#endif
#if 0
static int __init wl127x_vio_leakage_fix(void)
{
	int ret = 0;

	ret = gpio_request(OMAP_GPIO_BT_NSHUTDOWN, "wl127x_bten");
	if (ret < 0) {
		printk(KERN_ERR "wl127x_bten gpio_%d request fail",
						OMAP_GPIO_BT_NSHUTDOWN);
		goto fail;
	}

	gpio_direction_output(OMAP_GPIO_BT_NSHUTDOWN, 1);
	mdelay(10);
	gpio_direction_output(OMAP_GPIO_BT_NSHUTDOWN, 0);
	udelay(64);

	gpio_free(OMAP_GPIO_BT_NSHUTDOWN);

fail:
	return ret;
}
#endif
static void mod_clock_correction(void)
{
	cm_write_mod_reg(0x00,OMAP3430ES2_SGX_MOD,CM_CLKSEL);
	cm_write_mod_reg(0x04,OMAP3430_CAM_MOD,CM_CLKSEL);
}
int config_twl4030_resource_remap( void )
{
	int ret = 0;

	printk("Start config_twl4030_resource_remap\n");

	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_OFF, TWL4030_VDD1_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VDD1_REMAP\n");
	
	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_OFF, TWL4030_VDD2_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VDD2_REMAP\n");

	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_OFF, TWL4030_VPLL1_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VPLL1_REMAP\n");

	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_OFF, TWL4030_VPLL2_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VPLL2_REMAP\n");

	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_OFF, TWL4030_HFCLKOUT_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_HFCLKOUT_REMAP\n");

	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_OFF, TWL4030_VINTANA2_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VINTANA2_REMAP\n");

	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_SLEEP, TWL4030_VINTDIG_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VINTDIG_REMAP\n");

	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_ACTIVE, TWL4030_VSIM_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VSIM_REMAP\n");

	// [ - In order to prevent damage to the PMIC(TWL4030 or 5030), 
	//     VINTANA1 should maintain active state even though the system is in offmode.
	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, REMAP_ACTIVE, TWL4030_VINTANA1_REMAP );
	if (ret)
		printk(" board-file: fail to set reousrce remap TWL4030_VINTANA1_REMAP\n");
	// ]

#if 1//TI HS.Yoon 20101018 for increasing VIO level to 1.85V
	ret = twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, 0x1, TWL4030_VIO_VSEL );
	if (ret)
		printk(" board-file: fail to set TWL4030_VIO_VSEL\n");
#endif

	return ret;
}

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_OTG,
	.power			= 100,
};
#define POWERUP_REASON "androidboot.mode"
int get_powerup_reason(char *str)
{
	extern char *saved_command_line;
	char *next, *start = NULL;
	int i;

	i = strlen(POWERUP_REASON);
	next = saved_command_line;
    
	while ((next = strchr(next, 'a')) != NULL) {
		if (!strncmp(next, POWERUP_REASON, i)) {
			start = next;
			break;
		} else {
			next++;
		}
	}
	if (!start)
		return -EPERM;
	i = 0;
	start = strchr(start, '=') + 1;
	while (*start != ' ') {
		str[i++] = *start++;
		if (i > 14) {
			printk(KERN_INFO "Invalid Powerup reason\n");
			return -EPERM;
		}
	}
	str[i] = '\0';
	return 0;
}


#ifdef CONFIG_ARCHER_KOR_DEBUG// klaatu
typedef struct {
	char Magic[4];
	char BuildRev[12];
	char BuildDate[12];
	char BuildTime[9];
	void *Excp_reserve1;
	void *Excp_reserve2;
	void *Excp_reserve3;
	void *Excp_reserve4;
}gExcpDebugInfo_t;

void debug_info_init(void)
{
	gExcpDebugInfo_t *debug_info;
#if (CONFIG_ARCHER_REV >= ARCHER_REV11)
	debug_info = phys_to_virt(0x95000000) - sizeof(gExcpDebugInfo_t);
#else
	debug_info = phys_to_virt(0x8D000000) - sizeof(gExcpDebugInfo_t);
#endif
	memcpy(debug_info->Magic,"DBG",4);
	memcpy(debug_info->BuildRev,CONFIG_REV_STR,12);
	memcpy(debug_info->BuildDate,__DATE__,12);
	memcpy(debug_info->BuildTime,__TIME__,9);
}

#if 0//def CONFIG_ARCHER_KOR_DEBUG_USER
#define CONFIG_ARCHER_SEMAPHORE_LOGGING
#endif

#ifdef CONFIG_ARCHER_SEMAPHORE_LOGGING
extern void debug_for_semaphore_init(void);
#endif

#endif

#ifdef  CONFIG_ARCHER_TARGET_SK
#if (CONFIG_ARCHER_REV >= ARCHER_REV11)
static void __init get_board_hw_rev(void)
{
	int ret;

	ret = gpio_request( OMAP_GPIO_HW_REV0, "HW_REV0");
	if (ret < 0)
	{
		printk( "fail to get gpio : %d, res : %d\n", OMAP_GPIO_HW_REV0, ret );
		return;
	}

	ret = gpio_request( OMAP_GPIO_HW_REV1, "HW_REV1");
	if (ret < 0)
	{
		printk( "fail to get gpio : %d, res : %d\n", OMAP_GPIO_HW_REV1, ret );
		return;
	}

	ret = gpio_request( OMAP_GPIO_HW_REV2, "HW_REV2");
	if (ret < 0)
	{
		printk( "fail to get gpio : %d, res : %d\n", OMAP_GPIO_HW_REV2, ret );
		return;
	}

	gpio_direction_input( OMAP_GPIO_HW_REV0 );
	gpio_direction_input( OMAP_GPIO_HW_REV1 );
	gpio_direction_input( OMAP_GPIO_HW_REV2 );

	hw_revision = gpio_get_value(OMAP_GPIO_HW_REV0);
	hw_revision |= (gpio_get_value(OMAP_GPIO_HW_REV1) << 1);
	hw_revision |= (gpio_get_value(OMAP_GPIO_HW_REV2) << 2);

	gpio_free( OMAP_GPIO_HW_REV0 );
	gpio_free( OMAP_GPIO_HW_REV1 );
	gpio_free( OMAP_GPIO_HW_REV2 );

	// safe mode reconfiguration
	omap_ctrl_writew(OMAP34XX_MUX_MODE7, 0x018C);
	omap_ctrl_writew(OMAP34XX_MUX_MODE7, 0x0190);
	omap_ctrl_writew(OMAP34XX_MUX_MODE7, 0x0192);

	// REV03 : 000b, REV04 : 001b, REV05 : 010b, REV06 : 011b
	printk("BOARD HW REV : %d\n", hw_revision);
}
#endif
#endif
#if defined(CONFIG_ARCHER_TARGET_SK)
/* integer parameter get/set functions */
static ssize_t set_integer_param(param_idx idx, const char *buf, size_t size)
{
	int ret = 0;
	char *after;
	unsigned long value = simple_strtoul(buf, &after, 10);	
	unsigned int count = (after - buf);
	if (*after && isspace(*after))
		count++;
	if (count == size) {
		ret = count;

		if (sec_set_param_value)	sec_set_param_value(idx, &value);
	}

	return ret;
}
static ssize_t get_integer_param(param_idx idx, char *buf)
{
	int value;

	if (sec_get_param_value)	sec_get_param_value(idx, &value);

	return sprintf(buf, "%d\n", value);        
}
REGISTER_PARAM(__DEBUG_LEVEL, debug_level);
REGISTER_PARAM(__DEBUG_BLOCKPOPUP, block_popup); 
static void *dev_attr[] = {
	&dev_attr_debug_level,
	&dev_attr_block_popup,
};
static int archer_param_sysfs_init(void)
{
	int ret, i = 0;

	param_dev = device_create(sec_class, NULL, MKDEV(0,0), NULL, "param");

	if (IS_ERR(param_dev)) {
		pr_err("Failed to create device(param)!\n");
		return PTR_ERR(param_dev);	
	}

	for (; i < ARRAY_SIZE(dev_attr); i++) {
		ret = device_create_file(param_dev, dev_attr[i]);
		if (ret < 0) {
			pr_err("Failed to create device file(%s)!\n",
				((struct device_attribute *)dev_attr[i])->attr.name);
			goto fail;
		}
	}

	return 0;

fail:
	for (--i; i >= 0; i--) 
		device_remove_file(param_dev, dev_attr[i]);

	return -1;	
}

/* begins - andre.b.kim : added sec_setprio as module { */
static ssize_t sec_setprio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (sec_setprio_get_value) {
		sec_setprio_get_value();
	}
	
	return 0;
}

static ssize_t sec_setprio_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	if (sec_setprio_set_value) {
		sec_setprio_set_value(buf);
	}
	
	return size;
}

static DEVICE_ATTR(sec_setprio, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, sec_setprio_show, sec_setprio_store);

static int archer_sec_setprio_sysfs_init(void)
{
	/* file creation at '/sys/class/sec_setprio/sec_setprio  */
	sec_setprio_class = class_create(THIS_MODULE, "sec_setprio");
	if (IS_ERR(sec_setprio_class))
		pr_err("Failed to create class (sec_setprio)\n");

	sec_set_prio = device_create(sec_setprio_class, NULL, 0, NULL, "sec_setprio");
	if (IS_ERR(sec_set_prio))
		pr_err("Failed to create device(sec_set_prio)!\n");
	if (device_create_file(sec_set_prio, &dev_attr_sec_setprio) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_sec_setprio.attr.name);
	
	return 0;
}
/* } ends - andre.b.kim : added sec_setprio as module */
#endif
#define OMAP_GPIO_TSP_INT 186

static void Atmel_dev_init(void)
{
        /* Set the ts_gpio pin mux */
        if (gpio_request(OMAP_GPIO_TSP_INT, "touch_atmel") < 0) {
                printk(KERN_ERR "can't get synaptics pen down GPIO\n");
                return;
        }
        gpio_direction_input(OMAP_GPIO_TSP_INT);
}

void __init archer_peripherals_init(void)
{

	/*omap_i2c_init();*/ /*Moving omap_i2c_init to be called directly from archer init */  
         spi_register_board_info(archer_spi_board_info,
				ARRAY_SIZE(archer_spi_board_info));
//	platform_device_register(&archer_dss_device);
	Atmel_dev_init();
	omap_serial_init();
	config_wlan_gpio();
	
        config_camera_gpio();
        mod_clock_correction();
	usb_musb_init(&musb_board_data);
	//usb_ehci_init();  //Proper arguments should be passed
}



static void __init omap_archer_init(void)
{
	char str_powerup_reason[15];
	u32 regval;

	printk("\n.....[Archer] Initializing...\n");

	// change reset duration (PRM_RSTTIME register)
	regval = omap_readl(0x48307254);
	regval |= 0xFF;
	omap_writew(regval, 0x48307254);

	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);
	printk("Mux Setting Done\n");

	if( omap_gpio_out_init() ){
		printk( "zeus gpio ouput set fail!!!!\n" );
	}
	printk("Gpio Output Setting Done");

  set_wakeup_gpio();
	get_powerup_reason(str_powerup_reason);
	printk( "\n\n <Powerup Reason : %s>\n\n", str_powerup_reason);
	omap_i2c_init(); /*Moved here from peripheral init */
	platform_add_devices(archer_devices, ARRAY_SIZE(archer_devices));
// For Regulator Framework [
	archer_vaux1_supply.dev = &samsung_pl_sensor_power_device.dev;
	archer_vaux3_supply.dev = &archer_lcd_device.dev;
	archer_vaux4_supply.dev = &archer_lcd_device.dev;
	archer_vpll2_supply.dev = &archer_lcd_device.dev;
//	zeus_vmmc2_supply.dev = &zeus_lcd_device.dev; 
#if defined(CONFIG_ARCHER_TARGET_SK)
	archer_vmmc2_supply.dev = &samsung_led_device.dev; // ryun 20100216 for KEY LED driver
#endif
// ]
	archer_usb_data.batt_dev = &samsung_battery_device.dev;
	archer_usb_data.charger_dev = &samsung_charger_device.dev;
        archer_usb_data.switch_dev = &headset_switch_device.dev; //Added for Regulator


	omap_board_config = archer_config;
	omap_board_config_size = ARRAY_SIZE(archer_config);
	/*RTC support*/
	msecure_init();

#if  0 //(CONFIG_ARCHER_REV < ARCHER_REV10)
	synaptics_dev_init();
#endif
#ifdef CONFIG_MICROUSBIC_INTR
	microusbic_dev_init();
#endif
#ifdef CONFIG_ARCHER_KOR_DEBUG// klaatu
	debug_info_init();
#ifdef CONFIG_ARCHER_SEMAPHORE_LOGGING
    debug_for_semaphore_init();
#endif
#endif

#ifdef CONFIG_ARCHER_TARGET_SK
#if (CONFIG_ARCHER_REV >= ARCHER_REV11)
	get_board_hw_rev();
#endif
        sec_class = class_create(THIS_MODULE, "sec");
        if (IS_ERR(sec_class))
        {
                pr_err("Failed to create class(sec)!\n");
        }

	archer_param_sysfs_init();

	/* andre.b.kim : added sec_setprio as module */
	archer_sec_setprio_sysfs_init();
#endif

	archer_peripherals_init();
}


#ifdef CONFIG_ARCHER_TARGET_SK
static void __init bootloader_reserve_sdram(void)
{
        u32 paddr;
        u32 size = 0x80000;

#if (CONFIG_ARCHER_REV >= ARCHER_REV11)
        paddr = 0x95000000;
#else
        paddr = 0x8D000000;
#endif

        paddr -= size;

        if (reserve_bootmem(paddr, size, BOOTMEM_EXCLUSIVE) < 0) {
                pr_err("FB: failed to reserve VRAM\n");
        }
}
#endif


static void __init omap_archer_map_io(void)
{
	omap2_set_globals_343x();
	omap2_map_common_io();
#ifdef CONFIG_ARCHER_TARGET_SK
	bootloader_reserve_sdram();
#endif
}

static void __init omap_archer_fixup(struct machine_desc *desc,
					struct tag *tags, char **cmdline,
					struct meminfo *mi)
{
#ifdef CONFIG_ARCHER_TARGET_SK
#if (CONFIG_ARCHER_REV >= ARCHER_REV11)
	mi->bank[0].start = 0x80000000;
	mi->bank[0].size = 256 * SZ_1M; // DDR 256MB
	mi->bank[0].node = 0;	

	mi->bank[1].start = 0x90000000;
	mi->bank[1].size = 80 * SZ_1M; // OneDRAM 80MB
	mi->bank[1].node = 0;

	mi->nr_banks = 2;
#else
	mi->bank[0].start = 0x80000000;
	mi->bank[0].size = 128 * SZ_1M; // DDR 128MB
	mi->bank[0].node = 0;

	mi->bank[1].start = 0x88000000;
	mi->bank[1].size = 80 * SZ_1M; // OneDRAM 80MB
	mi->bank[1].node = 0;

	mi->nr_banks = 2;
#endif
#endif
}
MACHINE_START(ARCHER, "Archer Samsung board")
	.phys_io	= 0x48000000,
	.io_pg_offst	= ((0xfa000000) >> 18) & 0xfffc,
	.boot_params	= 0x80000100,
	.fixup		= omap_archer_fixup,
	.map_io		= omap_archer_map_io,
	.init_irq	= omap_archer_init_irq,
	.init_machine	= omap_archer_init,
	.timer		= &omap_timer,
MACHINE_END
