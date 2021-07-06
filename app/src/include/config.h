#ifndef _CONFIG_H_
#define _CONFIG_H_
/* This header must always be included first */
#include <zephyr.h>

#include <timing/timing.h>
#include <sys/printk.h>

#ifndef BASE_PRIO
#define BASE_PRIO 10
#endif

#define INITIALIZATION_TASK_RETVAL void
#define INITIALIZATION_TASK_ARG void
#define INITIALIZATION_TASK_NAME main

#define TASK_DEFAULT_RETURN
#define MAIN_DEFAULT_RETURN

#ifndef CONFIG_TRACING_BACKEND_UART
#ifndef TRACING
#ifndef NO_VERBOSE_RESULTS
#define REPORT_BENCHMARK_RESULTS(STR_PTR) \
	printk("%s\n", STR_PTR);\
	no_result_report(max_cycles, min_cycles, average_cycles);
#else
#define REPORT_BENCHMARK_RESULTS(STR_PTR) \
	printk("%s\n", STR_PTR);\
	for (int _i = 1; _i < NB_ITER; _i++) {\
		no_single_result_report("", no_cycles_results[_i]);\
	}
#endif /* !NO_VERBOSE_RESULTS */
#endif /* !TRACING */
#else
/* Prevent UART output from corrupting our trace stream */
#define REPORT_BENCHMARK_RESULTS(STR_PTR)
#endif /* !CONFIG_TRACING_BACKEND_UART */

#define DO_WORKLOAD(i) \

#define NO_DECREASE_TASK_PRIO(prio, decrease_by) (prio + decrease_by)

typedef void no_task_retval_t;
typedef void * no_task_argument_t;
typedef k_tid_t no_task_handle_t;
typedef no_task_retval_t (*no_task_entry_t)(no_task_argument_t);

typedef void no_main_retval_t;
typedef void* no_main_argument_t;

typedef timing_t       no_time_t;
typedef struct k_sem   no_sem_t;
typedef struct k_mutex no_mutex_t;
typedef struct event {
	struct k_mutex   mtx;
	struct k_condvar condvar;
} no_event_t;
typedef struct k_msgq  no_mq_t;
typedef void (*no_int_handler_t)(const void *parameter);

#define NO_DECLARE_INT_HANDLER(NAME, TIME_CNTR_SUFFIX)\
void NAME(const void *param) {\
	WRITE_T2_COUNTER(TIME_CNTR_SUFFIX)\
}

#endif /* _CONFIG_H_ */
