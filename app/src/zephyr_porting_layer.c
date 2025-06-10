#include <zephyr/sys/__assert.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/timing/timing.h>

#include "porting_layer.h"
#include "nvic.h"

void
no_initialize_test(no_task_entry_t init_function)
{
	uint32_t freq;

	timing_init();
	timing_start();

	freq = timing_freq_get_mhz();
#ifndef CONFIG_TRACING_BACKEND_UART
	printk("Time Measurement\n");
	printk("Timing results: Clock frequency: %u MHz\n", freq);
#endif /* !CONFIG_TRACING_BACKEND_UART */
#if defined(CONFIG_TRACING_HANDLE_HOST_CMD) || defined(CONFIG_SEGGER_SYSTEMVIEW)
	/*
	 * Sleep for five seconds so the tracing script can attach to
	 * us before we are doing anything useful.
	 */
	k_msleep(5000);
#endif /* CONFIG_TRACING_HANDLE_HOST_CMD || CONFIG_SEGGER_SYSTEMVIEW */
	k_sched_lock();
	/*
	 * By default threads do not have have an active resource pool.
	 * We assign the system heap as the main threads resource pool
	 * for simplicity's sake. All other threads should inherit this
	 * automatically.
	 */
	k_thread_system_pool_assign(k_current_get());
	init_function(NULL);
	k_sched_unlock();
}

static void
entry_wrapper(void *entry_point, void *unused1, void *unused32)
{
	no_task_entry_t ep = (no_task_entry_t)entry_point;
	ep(NULL);
}

#define NUM_THREADS 5
#define STACK_SIZE  (1024+NB_ITER*sizeof(int64_t))

/*
 * XXX: Threads are allocated statically. I have not yet found out
 * how/whether they can be allocated dynamically.
 */
static struct k_thread threads[NUM_THREADS];
static K_KERNEL_STACK_ARRAY_DEFINE(stacks, NUM_THREADS, STACK_SIZE);
static int num_active_threads = 0;

static K_MUTEX_DEFINE(task_mtx);

no_task_handle_t
no_create_task(no_task_entry_t task_entry, char task_name[4],
	       unsigned int prio)
{
	k_tid_t tid;

	k_mutex_lock(&task_mtx, K_FOREVER);
	if (num_active_threads >= NUM_THREADS) {
		k_panic();
	}

	tid = k_thread_create(&threads[num_active_threads],
	    stacks[num_active_threads],
	    K_THREAD_STACK_SIZEOF(stacks[num_active_threads]),
	    entry_wrapper,
	    task_entry, NULL, NULL,
	    (int)prio, 0, K_NO_WAIT);
#ifdef CONFIG_THREAD_NAME
	k_thread_name_set(tid, task_name);
#endif
	num_active_threads++;

	k_mutex_unlock(&task_mtx);

	return (tid);
}

void
no_task_yield()
{
	k_yield();
}

void
no_task_suspend(no_task_handle_t task)
{
	k_thread_suspend(task);
}

void
no_task_suspend_self(void)
{
}

void
no_task_resume(no_task_handle_t task)
{
	k_thread_resume(task);
}

void
no_task_delay(unsigned int milliseconds)
{
	k_msleep(milliseconds);
}

no_time_t
no_time_get()
{
	// return (timing_counter_get());
	return (no_time_t)DWT->CYCCNT;
}

no_time_t
no_add_times(const no_time_t* base, unsigned int ticks) {
    return (*base + ticks);
}

long
no_time_diff(const no_time_t* t1, const no_time_t* t2)
{
	no_time_t first;
	no_time_t second;
	uint64_t diff;

	first = *t1;
	second = *t2;
	diff = timing_cycles_get(&first, &second);
	__ASSERT(diff < (uint64_t)LONG_MAX, "Time difference does not fit into long");
	return ((long)diff);
}

void
no_cycle_reset_counter(void)
{
}

void
no_sem_create(no_sem_t* sem, int current_value)
{
	__ASSERT(current_value >= 0, "Invalid current_value (got %d)",
	    current_value);
	k_sem_init(sem, (unsigned int)current_value, K_SEM_MAX_LIMIT);
}

void
no_sem_wait(no_sem_t* sem)
{
	int err;
	do {
		err = k_sem_take(sem, K_FOREVER);
	} while (err == -EAGAIN);
	__ASSERT(!err, "Failed to take semaphore (got return value %d)", err);
}

void
no_sem_signal(no_sem_t* sem)
{
	k_sem_give(sem);
}

void
no_mutex_create(no_mutex_t* mtx)
{
	k_mutex_init(mtx);
}

void
no_mutex_acquire(no_mutex_t* mtx)
{
	k_mutex_lock(mtx, K_FOREVER);
}

void
no_mutex_release(no_mutex_t* mtx)
{
	k_mutex_unlock(mtx);
}

void
no_event_create(no_event_t* event)
{
	k_mutex_init(&event->mtx);
	k_condvar_init(&event->condvar);
}

void
no_event_wait(no_event_t* event)
{
	k_mutex_lock(&event->mtx, K_FOREVER);
	k_condvar_wait(&event->condvar, &event->mtx, K_FOREVER);
	k_mutex_unlock(&event->mtx);
}

void
no_event_set(no_event_t* event)
{
	k_mutex_lock(&event->mtx, K_FOREVER);
	k_condvar_broadcast(&event->condvar);
	k_mutex_unlock(&event->mtx);
}

void
no_event_reset(no_event_t* event)
{
}

void
no_mq_create(no_mq_t* mq, unsigned int length, unsigned int msgsize)
{
	int err;

	err = k_msgq_alloc_init(mq, msgsize, length);
	if (err) {
		k_panic();
	}
}

void
no_mq_send(no_mq_t* mq, unsigned int msg)
{
	int err;

	do {
		err = k_msgq_put(mq, &msg, K_FOREVER);
	} while (err == -ENOMSG || err == -EAGAIN);
}

void
no_mq_receive(no_mq_t* mq)
{
	int err;
	unsigned int msg;

	do {
		err = k_msgq_get(mq, &msg, K_FOREVER);
	} while (err == -ENOMSG || err == -EAGAIN);
	if (err) {
		k_panic();
	}
}

void
no_serial_write(const char* string)
{
	printk("%s\n", string);
}

void
no_result_report(int64_t max, int64_t min, int64_t average)
{
	uint64_t ns_max;
	uint64_t ns_min;
	uint64_t ns_average;

	printk("min = %lld cycles, max = %lld cycles, average = %lld cycles\n",
	    min, max, average);

	ns_min = timing_cycles_to_ns((uint64_t)min);
	ns_max = timing_cycles_to_ns((uint64_t)max);
	ns_average = timing_cycles_to_ns((uint64_t)average);
	printk("min = %llu ns, max = %llu ns, average = %llu ns\n",
	   ns_min, ns_max, ns_average);
}

void
no_single_result_report(char* prefix, int64_t time)
{
	printk("%s%lld\n", prefix, time);
}

#define INTERRUPT_NUM  6
#define INTERRUPT_PRIO 0xf

void
no_interrupt_register_sgi_handler(no_int_handler_t fn)
{
	irq_connect_dynamic(INTERRUPT_NUM, INTERRUPT_PRIO, fn, NULL, 0);
}

void
no_interrupt_do_sgi()
{
	nvic_trigger_interrupt(INTERRUPT_NUM);
}

void
no_interrupt_enable_sgi()
{
	irq_enable(INTERRUPT_NUM);
}

void
no_tracing_report()
{
}
