// include/asm-arm/mach-omap/usb.h

#ifndef	__ASM_ARCH_OMAP_USB_H
#define	__ASM_ARCH_OMAP_USB_H

#include <linux/platform_device.h>
#include <linux/usb/musb.h>
#include <plat/board.h>

#define OMAP3_HS_USB_PORTS	3

enum ehci_hcd_omap_mode {
	/* ULPI_BYPASS = 0 */
	EHCI_HCD_OMAP_MODE_ULPI_PHY,

	/* ULPI_BYPASS = 1, CHANMODE = 1 */
	EHCI_HCD_OMAP_MODE_UTMI_PHY_6PIN,       /* FSLSMODE = 0x0 */
	EHCI_HCD_OMAP_MODE_UTMI_PHY_6PIN_ALT,   /* FSLSMODE = 0x1 */
	EHCI_HCD_OMAP_MODE_UTMI_PHY_3PIN,       /* FSLSMODE = 0x2 */
	EHCI_HCD_OMAP_MODE_UTMI_PHY_4PIN,       /* FSLSMODE = 0x3 */
	EHCI_HCD_OMAP_MODE_UTMI_TLL_6PIN,       /* FSLSMODE = 0x4 */
	EHCI_HCD_OMAP_MODE_UTMI_TLL_6PIN_ALT,   /* FSLSMODE = 0x5 */
	EHCI_HCD_OMAP_MODE_UTMI_TLL_3PIN,       /* FSLSMODE = 0x6 */
	EHCI_HCD_OMAP_MODE_UTMI_TLL_4PIN,       /* FSLSMODE = 0x7 */
	EHCI_HCD_OMAP_MODE_UTMI_TLL_2PIN,       /* FSLSMODE = 0xA */
	EHCI_HCD_OMAP_MODE_UTMI_TLL_2PIN_ALT,   /* FSLbSMODE = 0xB */

	/* ULPI_BYPASS = 1, CHANMODE = 0 */
	EHCI_HCD_OMAP_MODE_ULPI_TLL_SDR,        /* ULPIDDRMODE = 0 */
	EHCI_HCD_OMAP_MODE_ULPI_TLL_DDR,        /* ULPIDDRMODE = 1 */
};

#define EHCI_HCD_OMAP_FLAG_ENABLED	(1<<0)
#define EHCI_HCD_OMAP_FLAG_NOBITSTUFF	(1<<1)
#define EHCI_HCD_OMAP_FLAG_AUTOIDLE	(1<<2)

struct ehci_hcd_omap_port_data {
	enum ehci_hcd_omap_mode		mode;

	u32				flags;

	int				reset_delay;

	int	(*startup)(struct platform_device *dev, int port);
	void	(*shutdown)(struct platform_device *dev, int port);
	void	(*reset)(struct platform_device *dev, int port, int reset);
	void	(*suspend)(struct platform_device *dev, int port, int suspend);
};

struct ehci_hcd_omap_platform_data {
	struct ehci_hcd_omap_port_data	port_data[OMAP3_HS_USB_PORTS];
};

/*-------------------------------------------------------------------------*/

#define OMAP1_OTG_BASE			0xfffb0400
#define OMAP1_UDC_BASE			0xfffb4000
#define OMAP1_OHCI_BASE			0xfffba000

#define OMAP2_OHCI_BASE			0x4805e000
#define OMAP2_UDC_BASE			0x4805e200
#define OMAP2_OTG_BASE			0x4805e300

#ifdef CONFIG_ARCH_OMAP1

#define OTG_BASE			OMAP1_OTG_BASE
#define UDC_BASE			OMAP1_UDC_BASE
#define OMAP_OHCI_BASE			OMAP1_OHCI_BASE

#else

#define OTG_BASE			OMAP2_OTG_BASE
#define UDC_BASE			OMAP2_UDC_BASE
#define OMAP_OHCI_BASE			OMAP2_OHCI_BASE

struct omap_musb_board_data {
	u8      interface_type;
	u8      mode;
	u8      power;
};

enum musb_interface    {MUSB_INTERFACE_ULPI, MUSB_INTERFACE_UTMI};

extern void usb_musb_init(struct omap_musb_board_data *board_data);

extern void usb_ehci_init(struct ehci_hcd_omap_platform_data *pdata);

/* This is needed for OMAP3 errata 1.164: enabled autoidle can prevent sleep */
extern void usb_musb_disable_autoidle(void);

#endif

void omap_usb_init(struct omap_usb_config *pdata);

/*-------------------------------------------------------------------------*/

/*
 * OTG and transceiver registers, for OMAPs starting with ARM926
 */
#define OTG_REV				(OTG_BASE + 0x00)
#define OTG_SYSCON_1			(OTG_BASE + 0x04)
#	define	 USB2_TRX_MODE(w)	(((w)>>24)&0x07)
#	define	 USB1_TRX_MODE(w)	(((w)>>20)&0x07)
#	define	 USB0_TRX_MODE(w)	(((w)>>16)&0x07)
#	define	 OTG_IDLE_EN		(1 << 15)
#	define	 HST_IDLE_EN		(1 << 14)
#	define	 DEV_IDLE_EN		(1 << 13)
#	define	 OTG_RESET_DONE		(1 << 2)
#	define	 OTG_SOFT_RESET		(1 << 1)
#define OTG_SYSCON_2			(OTG_BASE + 0x08)
#	define	 OTG_EN			(1 << 31)
#	define	 USBX_SYNCHRO		(1 << 30)
#	define	 OTG_MST16		(1 << 29)
#	define	 SRP_GPDATA		(1 << 28)
#	define	 SRP_GPDVBUS		(1 << 27)
#	define	 SRP_GPUVBUS(w)		(((w)>>24)&0x07)
#	define	 A_WAIT_VRISE(w)	(((w)>>20)&0x07)
#	define	 B_ASE_BRST(w)		(((w)>>16)&0x07)
#	define	 SRP_DPW		(1 << 14)
#	define	 SRP_DATA		(1 << 13)
#	define	 SRP_VBUS		(1 << 12)
#	define	 OTG_PADEN		(1 << 10)
#	define	 HMC_PADEN		(1 << 9)
#	define	 UHOST_EN		(1 << 8)
#	define	 HMC_TLLSPEED		(1 << 7)
#	define	 HMC_TLLATTACH		(1 << 6)
#	define	 OTG_HMC(w)		(((w)>>0)&0x3f)
#define OTG_CTRL			(OTG_BASE + 0x0c)
#	define	 OTG_USB2_EN		(1 << 29)
#	define	 OTG_USB2_DP		(1 << 28)
#	define	 OTG_USB2_DM		(1 << 27)
#	define	 OTG_USB1_EN		(1 << 26)
#	define	 OTG_USB1_DP		(1 << 25)
#	define	 OTG_USB1_DM		(1 << 24)
#	define	 OTG_USB0_EN		(1 << 23)
#	define	 OTG_USB0_DP		(1 << 22)
#	define	 OTG_USB0_DM		(1 << 21)
#	define	 OTG_ASESSVLD		(1 << 20)
#	define	 OTG_BSESSEND		(1 << 19)
#	define	 OTG_BSESSVLD		(1 << 18)
#	define	 OTG_VBUSVLD		(1 << 17)
#	define	 OTG_ID			(1 << 16)
#	define	 OTG_DRIVER_SEL		(1 << 15)
#	define	 OTG_A_SETB_HNPEN	(1 << 12)
#	define	 OTG_A_BUSREQ		(1 << 11)
#	define	 OTG_B_HNPEN		(1 << 9)
#	define	 OTG_B_BUSREQ		(1 << 8)
#	define	 OTG_BUSDROP		(1 << 7)
#	define	 OTG_PULLDOWN		(1 << 5)
#	define	 OTG_PULLUP		(1 << 4)
#	define	 OTG_DRV_VBUS		(1 << 3)
#	define	 OTG_PD_VBUS		(1 << 2)
#	define	 OTG_PU_VBUS		(1 << 1)
#	define	 OTG_PU_ID		(1 << 0)
#define OTG_IRQ_EN			(OTG_BASE + 0x10)	/* 16-bit */
#	define	 DRIVER_SWITCH		(1 << 15)
#	define	 A_VBUS_ERR		(1 << 13)
#	define	 A_REQ_TMROUT		(1 << 12)
#	define	 A_SRP_DETECT		(1 << 11)
#	define	 B_HNP_FAIL		(1 << 10)
#	define	 B_SRP_TMROUT		(1 << 9)
#	define	 B_SRP_DONE		(1 << 8)
#	define	 B_SRP_STARTED		(1 << 7)
#	define	 OPRT_CHG		(1 << 0)
#define OTG_IRQ_SRC			(OTG_BASE + 0x14)	/* 16-bit */
	// same bits as in IRQ_EN
#define OTG_OUTCTRL			(OTG_BASE + 0x18)	/* 16-bit */
#	define	 OTGVPD			(1 << 14)
#	define	 OTGVPU			(1 << 13)
#	define	 OTGPUID		(1 << 12)
#	define	 USB2VDR		(1 << 10)
#	define	 USB2PDEN		(1 << 9)
#	define	 USB2PUEN		(1 << 8)
#	define	 USB1VDR		(1 << 6)
#	define	 USB1PDEN		(1 << 5)
#	define	 USB1PUEN		(1 << 4)
#	define	 USB0VDR		(1 << 2)
#	define	 USB0PDEN		(1 << 1)
#	define	 USB0PUEN		(1 << 0)
#define OTG_TEST			(OTG_BASE + 0x20)	/* 16-bit */
#define OTG_VENDOR_CODE			(OTG_BASE + 0xfc)	/* 16-bit */

/*-------------------------------------------------------------------------*/

/* OMAP1 */
#define	USB_TRANSCEIVER_CTRL		(0xfffe1000 + 0x0064)
#	define	CONF_USB2_UNI_R		(1 << 8)
#	define	CONF_USB1_UNI_R		(1 << 7)
#	define	CONF_USB_PORT0_R(x)	(((x)>>4)&0x7)
#	define	CONF_USB0_ISOLATE_R	(1 << 3)
#	define	CONF_USB_PWRDN_DM_R	(1 << 2)
#	define	CONF_USB_PWRDN_DP_R	(1 << 1)

/* OMAP2 */
#	define	USB_UNIDIR			0x0
#	define	USB_UNIDIR_TLL			0x1
#	define	USB_BIDIR			0x2
#	define	USB_BIDIR_TLL			0x3
#	define	USBTXWRMODEI(port, x)	((x) << (22 - (port * 2)))
#	define	USBT2TLL5PI		(1 << 17)
#	define	USB0PUENACTLOI		(1 << 16)
#	define	USBSTANDBYCTRL		(1 << 15)

#endif	/* __ASM_ARCH_OMAP_USB_H */
