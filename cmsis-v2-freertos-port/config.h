#ifndef CONFIG_H_
#define CONFIG_H_

/*
 * Only include the CMSIS-RTOS v2 header.
 * Remove all direct FreeRTOS includes.
 */
#include "cmsis_os2.h"

#define ACTIVATE_SEMAPHORE_TEST 0
#define ACTIVATE_MUTEX_TEST 0
#define ACTIVATE_MQ_TEST 0
#define ACTIVATE_CONTEXT_SWITCH_TEST 0
#define ACTIVATE_INTERRUPT_TEST 0

// Choose API
#define USE_FREERTOS_NATIVE 0

#define NB_ITER 1500
#define NB_TASK 5
#define BASE_PRIO osPriorityNormal

/*
 * Return/argument types used by RTOSBench (no_ prefix).
 */
typedef void              no_task_retval_t;
typedef void *            no_task_argument_t;
typedef int               no_main_retval_t;
typedef int               no_main_argument_t;
typedef no_task_retval_t (*no_task_entry_t)(no_task_argument_t);

/*
 * CMSIS-RTOS v2 thread handle:
 */
typedef osThreadId_t      no_task_handle_t;

typedef uint32_t          no_time_t;

/*
 * Semaphore, Mutex, EventFlags, MessageQueue from CMSIS-RTOS v2:
 */
typedef osSemaphoreId_t   no_sem_t;
typedef osMutexId_t       no_mutex_t;
typedef osEventFlagsId_t  no_event_t;
typedef osMessageQueueId_t no_mq_t;

typedef void (*no_int_handler_t)(int, void*);

extern volatile unsigned int* coreMailboxInterruptClr;

#define NO_DECLARE_INT_HANDLER(NAME, TIME_CNTR_SUFFIX) \
    void NAME(int nIRQ, void *pParam) {              \
        WRITE_T2_COUNTER(TIME_CNTR_SUFFIX)            \
        *coreMailboxInterruptClr = 0x1;               \
    }

#define INITIALIZATION_TASK_RETVAL int
#define INITIALIZATION_TASK_ARG   void
#define INITIALIZATION_TASK_NAME  main

#define TASK_DEFAULT_RETURN
#define MAIN_DEFAULT_RETURN 0

#define TIMER_COUNT_TO_NS(x) ((x) / TIMER_FREQUENCY_GHZ)
#define NO_INIT_MAX_TIME_VALUE 0x6FFFFFFF

#define BITSPERLONG 32
#define TOP2BITS(x) (((x) & (3L << (BITSPERLONG-2))) >> (BITSPERLONG-2))

#define DO_WORKLOAD(i)                                    \
    do {                                                  \
        unsigned long x = 9;                              \
        unsigned long a = 0L;                             \
        unsigned long r = 0L;                             \
        unsigned long e = 0L;                             \
        int _workload_i_;                                 \
        for (_workload_i_ = 0; _workload_i_ < BITSPERLONG; _workload_i_++) { \
            r = (r << 2) + TOP2BITS(x); x <<= 2;           \
            a <<= 1;                                       \
            e = (a << 1) + 1;                              \
            if (r >= e) {                                  \
                r -= e;                                    \
                a++;                                       \
            }                                              \
        }                                                  \
        _workload_results[i] = a;                          \
    } while (0);


// ROUND_ROBIN CONTEXT_SWITCH
no_task_retval_t round_robin_stress_initialize_test(no_task_argument_t args);
no_task_retval_t round_robin_task(no_task_argument_t args);

// INTERRUPT PROECSSING
no_task_retval_t interrupt_initialize_test(no_task_argument_t args);
no_task_retval_t interrupt_processing_task(no_task_argument_t args);

// MQ
no_task_retval_t mq_initialize_test(no_task_argument_t args);
no_task_retval_t sender(no_task_argument_t args);
no_task_retval_t receiver(no_task_argument_t args);

// MUTEX
no_task_retval_t mutex_initialize_test(no_task_argument_t args);
no_task_retval_t sender(no_task_argument_t args);
no_task_retval_t receiver(no_task_argument_t args);

// JITTER
no_task_retval_t monitor(no_task_argument_t args);
no_task_retval_t task(no_task_argument_t args);
no_task_retval_t jitter_initialize_test(no_task_argument_t args);

// SEMAPHORE
no_task_retval_t sem_initialize_test(no_task_argument_t args);
no_task_retval_t sender(no_task_argument_t args);
no_task_retval_t receiver(no_task_argument_t args);

#endif /* CONFIG_H_ */


