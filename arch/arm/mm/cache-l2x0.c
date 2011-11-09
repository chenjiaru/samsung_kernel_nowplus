/*
 * arch/arm/mm/cache-l2x0.c - L210/L220 cache controller support
 *
 * Copyright (C) 2007 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/hardware/cache-l2x0.h>

#define CACHE_LINE_SIZE		32

static void __iomem *l2x0_base;
bool l2x0_disabled;

#ifdef CONFIG_CACHE_PL310
static inline void cache_wait(void __iomem *reg, unsigned long mask)
{
	/* cache operations are atomic */
}

#define l2x0_lock(lock, flags)		((void)(flags))
#define l2x0_unlock(lock, flags)	((void)(flags))

#define block_end(start, end)		(end)

#define L2CC_TYPE			"PL310/L2C-310"
#else
static inline void cache_wait(void __iomem *reg, unsigned long mask)
{
	/* wait for the operation to complete */
	while (readl(reg) & mask)
		;
}

static DEFINE_SPINLOCK(l2x0_lock);
#define l2x0_lock(lock, flags)		spin_lock_irqsave(lock, flags)
#define l2x0_unlock(lock, flags)	spin_unlock_irqrestore(lock, flags)

#define block_end(start, end)		((start) + min((end) - (start), 4096UL))

#define L2CC_TYPE			"L2x0"
#endif

static inline void cache_wait_always(void __iomem *reg, unsigned long mask)
{
	/* wait for the operation to complete */
	while (readl(reg) & mask)
		;
}

static inline void cache_sync(void)
{
	void __iomem *base = l2x0_base;
	writel(0, base + L2X0_CACHE_SYNC);
	cache_wait(base + L2X0_CACHE_SYNC, 1);
}

static inline void l2x0_clean_line(unsigned long addr)
{
	void __iomem *base = l2x0_base;
	cache_wait(base + L2X0_CLEAN_LINE_PA, 1);
	writel(addr, base + L2X0_CLEAN_LINE_PA);
}

static inline void l2x0_inv_line(unsigned long addr)
{
	void __iomem *base = l2x0_base;
	cache_wait(base + L2X0_INV_LINE_PA, 1);
	writel(addr, base + L2X0_INV_LINE_PA);
}

static void debug_writel(unsigned long val)
{
	extern void omap_smc1(u32 fn, u32 arg);

	/*
	 * Texas Instrument secure monitor api to modify the
	 * PL310 Debug Control Register.
	 */
	if (omap_rev() == OMAP4430_REV_ES1_0)
	omap_smc1(0x100, val);
}

static inline void l2x0_flush_line(unsigned long addr)
{
	void __iomem *base = l2x0_base;

	if (omap_rev() == OMAP4430_REV_ES1_0) {
	/* Clean by PA followed by Invalidate by PA */
	cache_wait(base + L2X0_CLEAN_LINE_PA, 1);
	writel(addr, base + L2X0_CLEAN_LINE_PA);
	cache_wait(base + L2X0_INV_LINE_PA, 1);
	writel(addr, base + L2X0_INV_LINE_PA);
	} else {
	cache_wait(base + L2X0_CLEAN_INV_LINE_PA, 1);
	writel(addr, base + L2X0_CLEAN_INV_LINE_PA);
}
}

static void l2x0_cache_sync(void)
{
	unsigned long flags;

	l2x0_lock(&l2x0_lock, flags);
	cache_sync();
	l2x0_unlock(&l2x0_lock, flags);
}

static inline void l2x0_inv_all(void)
{
	unsigned long flags;

	/* invalidate all ways */
	l2x0_lock(&l2x0_lock, flags);
	writel(0xff, l2x0_base + L2X0_INV_WAY);
	cache_wait_always(l2x0_base + L2X0_INV_WAY, 0xff);
	cache_sync();
	l2x0_unlock(&l2x0_lock, flags);
}

static void l2x0_inv_range(unsigned long start, unsigned long end)
{
	void __iomem *base = l2x0_base;
	unsigned long flags;

	l2x0_lock(&l2x0_lock, flags);
	if (start & (CACHE_LINE_SIZE - 1)) {
		start &= ~(CACHE_LINE_SIZE - 1);
		debug_writel(0x03);
		l2x0_flush_line(start);
		debug_writel(0x00);
		start += CACHE_LINE_SIZE;
	}

	if (end & (CACHE_LINE_SIZE - 1)) {
		end &= ~(CACHE_LINE_SIZE - 1);
		debug_writel(0x03);
		l2x0_flush_line(end);
		debug_writel(0x00);
	}

	while (start < end) {
		unsigned long blk_end = block_end(start, end);

		while (start < blk_end) {
			l2x0_inv_line(start);
			start += CACHE_LINE_SIZE;
		}

		if (blk_end < end) {
			l2x0_unlock(&l2x0_lock, flags);
			l2x0_lock(&l2x0_lock, flags);
		}
	}
	cache_wait(base + L2X0_INV_LINE_PA, 1);
	cache_sync();
	l2x0_unlock(&l2x0_lock, flags);
}

static void l2x0_clean_range(unsigned long start, unsigned long end)
{
	void __iomem *base = l2x0_base;
	unsigned long flags;

	l2x0_lock(&l2x0_lock, flags);
	start &= ~(CACHE_LINE_SIZE - 1);
	while (start < end) {
		unsigned long blk_end = block_end(start, end);

		while (start < blk_end) {
			l2x0_clean_line(start);
			start += CACHE_LINE_SIZE;
		}

		if (blk_end < end) {
			l2x0_unlock(&l2x0_lock, flags);
			l2x0_lock(&l2x0_lock, flags);
		}
	}
	cache_wait(base + L2X0_CLEAN_LINE_PA, 1);
	cache_sync();
	l2x0_unlock(&l2x0_lock, flags);
}

static void l2x0_flush_all(void)
{
	void __iomem *base = l2x0_base;
	unsigned char way;
	unsigned long flags, value;

	if (omap_rev() == OMAP4430_REV_ES1_0) {
		l2x0_lock(&l2x0_lock, flags);
		debug_writel(0x03);
		/* Clean all the ways */
		for (way = 0; way <= 0xf; way++, value = 0) {
			value = 1 << way;
			writel(value, base + L2X0_CLEAN_WAY);
			cache_wait_always(base + L2X0_CLEAN_WAY, value);
			cache_sync();
		}
		/* Invalidate all the ways */
		for (way = 0; way <= 0xf; way++, value = 0) {
			value = 1 << way;
			writel(value, base + L2X0_INV_WAY);
			cache_wait_always(base +  L2X0_INV_WAY, value);
			cache_sync();
		}
		debug_writel(0x00);
		l2x0_unlock(&l2x0_lock, flags);
	} else {
		/* invalidate all ways */
		spin_lock_irqsave(&l2x0_lock, flags);
		writel(0xff, l2x0_base + L2X0_CLEAN_INV_WAY);
		cache_wait(l2x0_base + L2X0_CLEAN_INV_WAY, 0xff);
		cache_sync();
		spin_unlock_irqrestore(&l2x0_lock, flags);
	}
}

static void l2x0_clean_all(void)
{
	void __iomem *base = l2x0_base;
	unsigned char way;
	unsigned long flags, value;

	if (omap_rev() == OMAP4430_REV_ES1_0) {
		l2x0_lock(&l2x0_lock, flags);
		debug_writel(0x03);
		/* Clean all the ways */
		for (way = 0; way <= 0xf; way++, value = 0) {
			value = 1 << way;
			writel(value, base + L2X0_CLEAN_WAY);
			cache_wait_always(base + L2X0_CLEAN_WAY, value);
			cache_sync();
		}
		debug_writel(0x00);
		l2x0_unlock(&l2x0_lock, flags);
	} else {
		/* invalidate all ways */
		spin_lock_irqsave(&l2x0_lock, flags);
		writel(0xff, l2x0_base + L2X0_CLEAN_WAY);
		cache_wait(l2x0_base + L2X0_CLEAN_WAY, 0xff);
		cache_sync();
		spin_unlock_irqrestore(&l2x0_lock, flags);
	}
}

static void l2x0_flush_range(unsigned long start, unsigned long end)
{
	void __iomem *base = l2x0_base;
	unsigned long flags;

	l2x0_lock(&l2x0_lock, flags);
	start &= ~(CACHE_LINE_SIZE - 1);
	while (start < end) {
		unsigned long blk_end = block_end(start, end);

		debug_writel(0x03);
		while (start < blk_end) {
			l2x0_flush_line(start);
			start += CACHE_LINE_SIZE;
		}
		debug_writel(0x00);

		if (blk_end < end) {
			l2x0_unlock(&l2x0_lock, flags);
			l2x0_lock(&l2x0_lock, flags);
		}
	}
	cache_wait(base + L2X0_CLEAN_INV_LINE_PA, 1);
	cache_sync();
	l2x0_unlock(&l2x0_lock, flags);
}

void __init l2x0_init(void __iomem *base, __u32 aux_val, __u32 aux_mask)
{
	__u32 aux;

	if (l2x0_disabled) {
		pr_info(L2CC_TYPE " cache controller disabled\n");
		return;
	}

	l2x0_base = base;

	/*
	 * Check if l2x0 controller is already enabled.
	 * If you are booting from non-secure mode
	 * accessing the below registers will fault.
	 */
	if (!(readl(l2x0_base + L2X0_CTRL) & 1)) {

		/* l2x0 controller is disabled */

		aux = readl(l2x0_base + L2X0_AUX_CTRL);
		aux &= aux_mask;
		aux |= aux_val;
		writel(aux, l2x0_base + L2X0_AUX_CTRL);

		l2x0_inv_all();

		/* enable L2X0 */
		writel(1, l2x0_base + L2X0_CTRL);
	}

	outer_cache.inv_range = l2x0_inv_range;
	outer_cache.clean_range = l2x0_clean_range;
	outer_cache.flush_range = l2x0_flush_range;
	outer_cache.sync = l2x0_cache_sync;
	outer_cache.flush_all = l2x0_flush_all;
	outer_cache.clean_all = l2x0_clean_all;

	pr_info(L2CC_TYPE " cache controller enabled\n");
}

static int __init l2x0_disable(char *unused)
{
	l2x0_disabled = 1;
	return 0;
}
early_param("nol2x0", l2x0_disable);
