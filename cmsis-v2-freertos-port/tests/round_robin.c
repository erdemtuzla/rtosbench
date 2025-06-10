/*
 * round_robin.c
 *
 *  Created on: Jun 6, 2025
 *      Author: erdem
 */
#include "porting_layer.h"

#if ACTIVATE_CONTEXT_SWITCH_TEST

DECLARE_TIME_COUNTERS(no_time_t, _);

no_task_handle_t tasks_handle[NB_TASK];
char tasks_name[NB_TASK][5];

volatile int tasks_idx;
volatile int tasks_done_count;



no_task_retval_t round_robin_stress_initialize_test(no_task_argument_t args)
{
	int32_t i;

	tasks_idx = 0;
	tasks_done_count = 0;


	for (i = 0 ; i < NB_TASK; i++)
	{
		tasks_name[i][0] = 65;
		tasks_name[i][1] = (65 + i) % 255;
		tasks_name[i][2] = (66 + i) % 255;
		tasks_name[i][3] = (67 + i) % 255;
		tasks_name[i][4] = '\0';
		tasks_handle[i] = no_create_task(round_robin_task, tasks_name[i], BASE_PRIO);
	}

	no_task_suspend_self();
	return TASK_DEFAULT_RETURN;
}

no_task_retval_t round_robin_task(no_task_argument_t args)
{
	int32_t i;
	int32_t local_idx;

	local_idx = tasks_idx++;
	if (local_idx == 0)
	{
		DECLARE_TIME_STATS(int64_t);
		for (i = 0; i < NB_ITER-1; i++)
		{
			//DO_WORKLOAD(i)
			WRITE_T1_COUNTER(_);
			no_task_yield();
			if (i > 0)
			{
				COMPUTE_TIME_STATS(_, i);
			}
		}
		REPORT_BENCHMARK_RESULTS("-- round robin context switch results --");
	}
	else if (local_idx == 1)
	{
		for (i = 0; i < NB_ITER-1; i++)
		{
			//DO_WORKLOAD(i)
			no_task_yield();
			WRITE_T2_COUNTER(_);
		}
	}
	else
	{
		for (i = 0; i < NB_ITER; i++)
		{
			//DO_WORKLOAD(i)
			no_task_yield();
		}
	}

	no_task_suspend_self();

	return TASK_DEFAULT_RETURN;
}

#endif
