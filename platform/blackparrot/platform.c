/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 */


/*
 * Include these files as needed.
 * See config.mk PLATFORM_xxx configuration parameters.
 */
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_platform.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/sys/clint.h>
#include <sbi_utils/serial/sifive-uart.h>
#include <sbi/sbi_hart.h>

#define BP_PLIC_ADDR			0x10000000
#define BP_PLIC_NUM_SOURCES		0x35
#define BP_PLIC_NUM_PRIORITIES	7

#define BP_UART_ADDR		0x54000000
#define BP_UART_BAUDRATE		115200

#define BP_HART_COUNT			       1
#define BP_HART_STACK_SIZE		8192
#define BP_CLINT_ADDR			0x00300000



/*
 * Platform early initialization.
 */
static int blackparrot_early_init(bool cold_boot)
{
	return 0;
}


/*
 * Platform final initialization.
 */
static int blackparrot_final_init(bool cold_boot)
{
	void *fdt;

	if (!cold_boot)
		return 0;

	fdt = sbi_scratch_thishart_arg1_ptr();
	plic_fdt_fixup(fdt, "riscv,plic0");

  return 0;
}


/*
 * Get number of PMP regions for given HART.
 */
static u32 blackparrot_pmp_region_count(u32 hartid)
{
	return 1;
}


/*
 * Get PMP regions details (namely: protection, base address, and size) for
 * a given HART.
 */
static int blackparrot_pmp_region_info(u32 hartid, u32 index,
				    ulong *prot, ulong *addr, ulong *log2size)
{
  int ret = 0;

	switch (index) {
	case 0:
		*prot	  = PMP_R | PMP_W | PMP_X;
		*addr	  = 0;
		*log2size = __riscv_xlen;
		break;
	default:
		ret = -1;
		break;
	};

	return ret;
}


/*
 * Initialize the platform console.
 */
static int blackparrot_console_init(void)
{
  return sifive_uart_init(BP_UART_ADDR, 10000000, BP_UART_BAUDRATE);
}

/*
 * Write a character to the platform console output.
 
static void blackparrot_console_putc(char ch)
{
  sifive_uart_putc(ch);
}
*/


/*
 * Read a character from the platform console input.
 
static int blackparrot_console_getc(void)
{
	return sifive_uart_getc();
}
*/


/*
 * Initialize the platform interrupt controller for current HART.
 */
static int blackparrot_irqchip_init(bool cold_boot)
{

	int rc;
	u32 hartid = sbi_current_hartid();

	if (cold_boot) {
		rc = plic_cold_irqchip_init(
			BP_PLIC_ADDR, BP_PLIC_NUM_SOURCES, BP_HART_COUNT);
		if (rc)
			return rc;
	}

	return plic_warm_irqchip_init(hartid, (2 * hartid), (2 * hartid + 1));
}


/*
 * Initialize IPI for current HART.
 */
static int blackparrot_ipi_init(bool cold_boot)
{
	int rc;
  	if (cold_boot) {
		rc = clint_cold_ipi_init(BP_CLINT_ADDR, BP_HART_COUNT);
		if (rc)
			return rc;
}

	return clint_warm_ipi_init();
}


/*
 * Send IPI to a target HART
 
static void blackparrot_ipi_send(u32 target_hart)
{
	// Example if the generic CLINT driver is used 
	clint_ipi_send(target_hart);
}
*/

/*
 * Clear IPI for a target HART.
 
static void blackparrot_ipi_clear(u32 target_hart)
{
	// Example if the generic CLINT driver is used
	clint_ipi_clear(target_hart);
}
*/


/*
 * Initialize platform timer for current HART.
 */
static int blackparrot_timer_init(bool cold_boot)
{
  int rc;
  if (cold_boot) {
    rc = clint_cold_timer_init(BP_CLINT_ADDR, BP_HART_COUNT, TRUE);
		if (rc)
			return rc;
	}

	return clint_warm_timer_init();
}

/*
 * Get platform timer value.
 
static u64 platform_timer_value(void)
{
	// Example if the generic CLINT driver is used 
	return clint_timer_value();
}
*/

/*
 * Start platform timer event for current HART.
 
static void platform_timer_event_start(u64 next_event)
{
	// Example if the generic CLINT driver is used 
	clint_timer_event_start(next_event);
}
*/

/*
 * Stop platform timer event for current HART.

static void platform_timer_event_stop(void)
{
	// Example if the generic CLINT driver is used
	clint_timer_event_stop();
}
*/


/*
 * Reboot the platform.
 */
static int blackparrot_system_reboot(u32 type)
{
	return 0;
}

/*
 * Shutdown or poweroff the platform.
 */
static int blackparrot_system_shutdown(u32 type)
{
	return 0;
}


/*
 * Platform descriptor.
 */
const struct sbi_platform_operations platform_ops = {
  .pmp_region_count= blackparrot_pmp_region_count,
  .pmp_region_info= blackparrot_pmp_region_info,
  .early_init= blackparrot_early_init,
  .final_init= blackparrot_final_init,
  .console_putc= sifive_uart_putc, //add blackparrot_console_putc
  .console_getc= sifive_uart_getc,//add blackparrot_console_getc
  .console_init= blackparrot_console_init,
  .irqchip_init= blackparrot_irqchip_init,
  .ipi_send= clint_ipi_send,//need blackparrot_ipi_send?
  .ipi_clear= clint_ipi_clear,//need blackparrot_ipi_clear?
  .ipi_init= blackparrot_ipi_init,
  .timer_value= clint_timer_value,
  .timer_event_stop= clint_timer_event_stop, 
  .timer_event_start= clint_timer_event_start, 
  .timer_init= blackparrot_timer_init,
  .system_reboot= blackparrot_system_reboot,
  .system_shutdown= blackparrot_system_shutdown//change to BP finish func
};

const struct sbi_platform platform = {
  .opensbi_version= OPENSBI_VERSION,
  .platform_version= SBI_PLATFORM_VERSION(0x0, 0x01),
  .name= "blackparrot",
  .features= SBI_PLATFORM_DEFAULT_FEATURES,
  .hart_count= BP_HART_COUNT,
  .hart_stack_size= BP_HART_STACK_SIZE,
  .disabled_hart_mask= 0,
  .platform_ops_addr= (unsigned long)&platform_ops
};



