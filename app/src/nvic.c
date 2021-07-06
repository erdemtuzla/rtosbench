#include "stdint.h"

/**
 * \brief Software Trigger Interrupt Register (STIR).
 *
 * This register allows software to trigger arbitrary interrupts.
 */
struct nvic_sti_reg {
	uint32_t interrupt_id : 8,
		        : 24; /* reserved bits */
} __attribute__((packed));

volatile struct nvic_sti_reg *STIR = (volatile struct nvic_sti_reg*)0xe000ef00;

void nvic_trigger_interrupt(uint8_t id)
{
	struct nvic_sti_reg val;

	val = *STIR;
	val.interrupt_id = id;
	*STIR = val;
}

/*
 * Interrupt Set-enable Registers (ISER).
 *
 * The Interrupt Set-enable Registers are used to enable specific interrupt
 * lines and show which interrupts are enabled.
 */
struct nvic_iser_reg {
	uint32_t regs[8];
} __attribute__((packed));
