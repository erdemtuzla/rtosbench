/*
 * interrupt_processing.c
 *
 *  Created on: Jun 6, 2025
 *      Author: erdem
 */
#include "porting_layer.h"
#include "main.h"

#if ACTIVATE_INTERRUPT_TEST

no_task_handle_t tasks_handle;

DECLARE_TIME_COUNTERS(no_time_t, _);

NO_DECLARE_INT_HANDLER(swi_int_handler, _)

no_task_retval_t interrupt_initialize_test(no_task_argument_t args)
{
	tasks_handle = no_create_task(interrupt_processing_task, "t1", BASE_PRIO);
	no_interrupt_register_sgi_handler(swi_int_handler);
	no_interrupt_enable_sgi();

	no_task_suspend_self();
	return TASK_DEFAULT_RETURN;
}

no_task_retval_t interrupt_processing_task(no_task_argument_t args)
{
	int32_t i;
	DECLARE_TIME_STATS(int64_t)

#ifndef TRACING
		no_cycle_reset_counter();
#endif

	for (i = 0; i < NB_ITER; i++)
	{
		WRITE_T1_COUNTER(_)
		no_interrupt_do_sgi();
		COMPUTE_TIME_STATS(_, i);
#ifndef TRACING
		no_cycle_reset_counter();
#endif
	}

	REPORT_BENCHMARK_RESULTS("-- interrupt processing --");

	no_task_suspend_self();

	return TASK_DEFAULT_RETURN;
}

void EXTI4_IRQHandler(void)
{
    // Capture your “after‐interrupt” timestamp here
    WRITE_T2_COUNTER(_);

    // Clear the EXTI4 pending bit so it can fire again later
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
}

#endif

