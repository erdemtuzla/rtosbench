#ifndef PORTING_LAYER_H_
#define PORTING_LAYER_H_

#include "config.h"

/*-------------------------------------------------------
 * Timing and workload configuration
 *------------------------------------------------------*/

#ifndef NO_PART_PERIOD_NS
  #define NO_PART_PERIOD_NS 100000000
#endif

#ifndef NO_PART_RUNTIME_NS
  #define NO_PART_RUNTIME_NS 100000000
#endif

#define NO_PART_MAX_GAP 100000000

#ifndef NO_INIT_MIN_TIME_VALUE
  #define NO_INIT_MIN_TIME_VALUE 0
#endif

#ifndef NO_INIT_MAX_TIME_VALUE
  #define NO_INIT_MAX_TIME_VALUE 0x7FFFFFFFFFFFFFFF
#endif

#ifndef NO_INIT_TIME_STATS_AVERAGE
  #define NO_INIT_TIME_STATS_AVERAGE(avg, cyc, n) \
      ((long)avg + (((long)cyc - (long)avg) / ((n) + 1)))
#endif

/*-------------------------------------------------------
 * Workload macro
 *------------------------------------------------------*/

#ifndef DO_WORKLOAD
  #define BITSPERLONG 64
  #define TOP2BITS(x) (( (x) & (3L << (BITSPERLONG-2)) ) >> (BITSPERLONG-2))

  #define DO_WORKLOAD(i) \
    do { \
      unsigned long x = 9; \
      unsigned long a = 0L; \
      unsigned long r = 0L; \
      unsigned long e = 0L; \
      int _workload_i_; \
      for (_workload_i_ = 0; _workload_i_ < BITSPERLONG; _workload_i_++) { \
        r = (r << 2) + TOP2BITS(x);  x <<= 2; \
        a <<= 1;  \
        e = (a << 1) + 1; \
        if (r >= e) { \
          r -= e;  \
          a++; \
        } \
      } \
      _workload_results[i] = a; \
    } while (0);
#endif // DO_WORKLOAD

/*-------------------------------------------------------
 * Timing statistics macros
 *------------------------------------------------------*/

#ifndef NO_VERBOSE_RESULTS
  #define DECLARE_TIME_STATS(TYPE) \
    TYPE cycles; \
    TYPE max_cycles = NO_INIT_MIN_TIME_VALUE; \
    TYPE min_cycles = NO_INIT_MAX_TIME_VALUE; \
    TYPE average_cycles = NO_INIT_MIN_TIME_VALUE; \
    unsigned num_measurements = 0;

  #define COMPUTE_TIME_STATS(SUFFIX, i) \
    do { \
      cycles = no_time_diff(&SUFFIX##_t1, &SUFFIX##_t2); \
      if ( (cycles) < NO_PART_MAX_GAP && (cycles) > 0) { \
        if ((cycles) > max_cycles) max_cycles = (cycles); \
        if ((cycles) < min_cycles) min_cycles = (cycles); \
        average_cycles += ((cycles) - average_cycles) / (num_measurements + 1); \
        num_measurements++; \
      } \
    } while(0);

  #define RESET_TIME_STATS() \
    do { \
      average_cycles = 0; \
      max_cycles = 0; \
      min_cycles = NO_INIT_MAX_TIME_VALUE; \
      num_measurements = 0; \
    } while(0);

#else // NO_VERBOSE_RESULTS
  #define DECLARE_TIME_STATS(TYPE) \
    TYPE cycles; \
    TYPE no_cycles_results[NB_ITER];

  #define COMPUTE_TIME_STATS(SUFFIX, i) \
    do { \
      cycles = no_time_diff(&SUFFIX##_t1, &SUFFIX##_t2); \
      no_cycles_results[i] = cycles; \
    } while(0);

  #define RESET_TIME_STATS()
#endif // NO_VERBOSE_RESULTS

#define WRITE_T1_COUNTER(SUFFIX) \
	SUFFIX##_t1 = no_time_get();

#define WRITE_T2_COUNTER(SUFFIX) \
	SUFFIX##_t2 = no_time_get();

#ifndef DECLARE_TIME_COUNTERS
#define DECLARE_TIME_COUNTERS(TYPE, SUFFIX) \
    TYPE SUFFIX##_t1;                       \
    TYPE SUFFIX##_t2
#endif

#ifndef REPORT_BENCHMARK_RESULTS
  #ifndef NO_VERBOSE_RESULTS
    #define REPORT_BENCHMARK_RESULTS(STR_PTR)               \
      do {                                                  \
        printf("%s\n", STR_PTR);                            \
        printf("Measurement count: %d\n", num_measurements);\
        no_result_report((long)max_cycles, (long)min_cycles, (long)average_cycles); \
      } while (0)
  #else
    #define REPORT_BENCHMARK_RESULTS(STR_PTR)               \
      do {                                                  \
        printf("%s\n", STR_PTR);                            \
        for (int _i = 0; _i < NB_ITER; _i++) {               \
          printf("%s%lld\n", "", (long long)no_cycles_results[_i]); \
        }                                                   \
      } while (0)
  #endif /* NO_VERBOSE_RESULTS */
#endif /* REPORT_BENCHMARK_RESULTS */

/*-------------------------------------------------------
 * Task priority reduction macro
 *------------------------------------------------------*/

#ifndef NO_DECREASE_TASK_PRIO
  #define NO_DECREASE_TASK_PRIO(prio, decrease_by) ((prio) - (decrease_by))
#endif

/*-------------------------------------------------------
 * Porting API: CMSIS v2
 *------------------------------------------------------*/

/**
 * @brief   Initialize and launch a specific RTOSBench test case.
 *          Creates a single “init” thread that calls init_test_fct(), then starts the kernel.
 */
void no_initialize_test(no_task_entry_t init_test_fct);

/**
 * @brief   Create a new thread with given name and priority.
 *          In CMSIS v2, priority maps to osPriority_t.
 *
 * @param   task_entry  Function pointer to the thread’s entry point.
 * @param   task_name   4-character (or fewer) thread name. CMSIS v2 supports up to 8 chars.
 * @param   prio        Thread priority (castable to osPriority_t).
 * @return  Thread handle (osThreadId_t).
 */
no_task_handle_t no_create_task(no_task_entry_t task_entry,
                                char task_name[4],
                                unsigned int prio);

/**
 * @brief   Yield the CPU (give up remaining time slice).
 */
void no_task_yield(void);

/**
 * @brief   Terminate (exit) the calling thread.
 */
void no_task_suspend_self(void);

/**
 * @brief   Delay the calling thread by the specified number of milliseconds.
 */
void no_task_delay(unsigned int milliseconds);

void no_task_delay_until(no_time_t* last_wake, unsigned int ms);

/**
 * @brief   Read the current time value (in whatever units you chose).
 */
no_time_t no_time_get(void);

/**
 * @brief   Compute t2 - t1 (difference between two no_time_t values).
 */
long no_time_diff(const no_time_t *t1, const no_time_t *t2);

/**
 * @brief   Add a delta (in the same units as no_time_get()) to a base time.
 * @param   base   Pointer to the base timestamp
 * @param   ticks  Delta to add (in cycles)
 * @return  New timestamp = *base + ticks
 */
no_time_t no_add_times(const no_time_t *base, unsigned int ticks);

/**
 * @brief   Print final benchmark results (max, min, average).
 *          You must implement this to route output out (e.g., via UART).
 */
void no_result_report(int64_t max, int64_t min, int64_t average);

/**
 * @brief   Write a single timing sample with an optional prefix string.
 */
void no_single_result_report(char *prefix, int64_t time);

/**
 * @brief   Create a binary semaphore (max_count = 1).
 *          If current_value > 0, it starts “available”; otherwise it starts “taken.”
 */
void no_sem_create(no_sem_t *sem, int current_value);

/**
 * @brief   Acquire (wait) on the given semaphore (blocks forever if not available).
 */
void no_sem_wait(no_sem_t *sem);

/**
 * @brief   Release (signal) the given semaphore.
 */
void no_sem_signal(no_sem_t *sem);

/**
 * @brief   Create a recursive mutex.
 */
void no_mutex_create(no_mutex_t *mutex);

/**
 * @brief   Acquire the mutex (blocks until available).
 */
void no_mutex_acquire(no_mutex_t *mutex);

/**
 * @brief   Release the mutex.
 */
void no_mutex_release(no_mutex_t *mutex);

/**
 * @brief   Create an event‐flag group.
 */
void no_event_create(no_event_t *event);

/**
 * @brief   Set event bit(s) in the group.
 */
void no_event_set(no_event_t *event);

/**
 * @brief   Wait for the specified event bit(s) to be set (blocks forever).
 */
void no_event_wait(no_event_t *event);

/**
 * @brief   Clear the specified event bit(s).
 */
void no_event_reset(no_event_t *event);

/**
 * @brief   Create a message queue capable of 'length' messages of size 'msgsize' bytes.
 */
void no_mq_create(no_mq_t *mq, unsigned int length, unsigned int msgsize);

/**
 * @brief   Put a message (an unsigned int) into the queue (blocks forever if full).
 */
void no_mq_send(no_mq_t *mq, unsigned int msg);

/**
 * @brief   Receive a message from the queue (blocks forever if empty).
 */
void no_mq_receive(no_mq_t *mq);

/**
 * @brief   (Optional) Write a null‐terminated string to your serial console.
 */
void no_serial_write(const char *string);

/**
 * @brief   Generate a software‐generated interrupt (SGI).
 */
void no_interrupt_do_sgi(void);

/**
 * @brief   Enable or configure the SGI (if needed).
 */
void no_interrupt_enable_sgi(void);

/**
 * @brief   Register the SGI handler function at runtime.
 */
void no_interrupt_register_sgi_handler(no_int_handler_t fn);

/**
 * @brief   Reset hardware cycle counters (if you’re using DWT cycle counter).
 */
void no_cycle_reset_counter(void);

#ifdef TRACING
/**
 * @brief   If you built RTOSBench in trace mode, this will produce a trace‐buffer report.
 */
void no_tracing_report(void);
#endif // TRACING

#endif /* PORTING_LAYER_H_ */
