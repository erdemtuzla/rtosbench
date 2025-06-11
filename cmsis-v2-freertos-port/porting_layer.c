/*
 * porting_layer.c
 *
 *  Created on: Jun 5, 2025
 *      Author: erdem
 *
 *  Switchable between CMSIS-RTOS2 and native FreeRTOS APIs
 */

#include "porting_layer.h"
#include "stm32f4xx_hal.h"


#if USE_FREERTOS_NATIVE

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#else // CMSIS-RTOS2

#include "cmsis_os2.h"
#endif

#include <stdio.h>
#include <string.h>
#include "main.h"


volatile unsigned int* coreMailboxInterruptCtrl = (volatile unsigned int*)0x40000050;
volatile unsigned int* coreMailboxInterrupt = (volatile unsigned int*)0x40000080;
volatile unsigned int* coreMailboxInterruptClr = (volatile unsigned int*)0x400000C0;

// -- Initialization ---------------------------------------------------------
void no_initialize_test(no_task_entry_t init_test_fct)
{
    osKernelInitialize();
    osThreadAttr_t attr = { .name = "rtosbench_init",
                            .priority = osPriorityNormal,
                            .stack_size = 1024 };
    osThreadNew((osThreadFunc_t)init_test_fct, NULL, &attr);
    osKernelStart();
}

// -- Thread Management ------------------------------------------------------
no_task_handle_t no_create_task(no_task_entry_t task_entry, char task_name[4], unsigned int prio)
{
    osThreadAttr_t attr = { .name = (const char* const)task_name,
                            .stack_size = 128*8,
                            .priority = (osPriority_t)prio };
    return osThreadNew((osThreadFunc_t)task_entry, NULL, &attr);
}

void no_task_yield(void)
{
#if USE_FREERTOS_NATIVE
    taskYIELD();
#else
    osThreadYield();
#endif
}

void no_task_delay(unsigned int ms)
{
#if USE_FREERTOS_NATIVE
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    osDelay(ms);
#endif
}

void no_task_suspend_self(void)
{
#if USE_FREERTOS_NATIVE
    vTaskDelete(NULL);
#else
    osThreadExit();
#endif
}

// -- Time Measurement -------------------------------------------------------
void no_cycle_reset_counter(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
}

uint32_t no_cycle_get_count(void)
{
    return DWT->CYCCNT;
}

no_time_t no_time_get(void)
{
    return (no_time_t)DWT->CYCCNT;
}

no_time_t no_add_times(const no_time_t *base, unsigned int milliseconds)
{
    return (no_time_t)(*base + (no_time_t)milliseconds);
}

long no_time_diff(const no_time_t *t1, const no_time_t *t2)
{
    return (int64_t)(*t2) - (int64_t)(*t1);
}

// -- Semaphores & Mutexes --------------------------------------------------

void no_sem_create(no_sem_t *sem, int initial)
{
#if USE_FREERTOS_NATIVE
    SemaphoreHandle_t h = xSemaphoreCreateBinary();
    configASSERT(h);
    if (initial > 0) xSemaphoreGive(h);
    *sem = h;
#else
    *sem = osSemaphoreNew(1, initial>0?1:0, NULL);
#endif
}

void no_sem_wait(no_sem_t *sem)
{
#if USE_FREERTOS_NATIVE
    configASSERT(xSemaphoreTake(*sem, portMAX_DELAY) == pdTRUE);
#else
    osSemaphoreAcquire(*sem, osWaitForever);
#endif
}

void no_sem_signal(no_sem_t *sem)
{
#if USE_FREERTOS_NATIVE
    configASSERT(xSemaphoreGive(*sem) == pdTRUE);
#else
    osSemaphoreRelease(*sem);
#endif
}

void no_mutex_create(no_mutex_t *mtx)
{
#if USE_FREERTOS_NATIVE
    SemaphoreHandle_t m = xSemaphoreCreateMutex();
    configASSERT(m);
    *mtx = m;
#else
    osMutexAttr_t attr = {0};
    *mtx = osMutexNew(&attr);
#endif
}

void no_mutex_acquire(no_mutex_t *mtx)
{
#if USE_FREERTOS_NATIVE
    configASSERT(xSemaphoreTake(*mtx, portMAX_DELAY) == pdTRUE);
#else
    osMutexAcquire(*mtx, osWaitForever);
#endif
}

void no_mutex_release(no_mutex_t *mtx)
{
#if USE_FREERTOS_NATIVE
    configASSERT(xSemaphoreGive(*mtx) == pdTRUE);
#else
    osMutexRelease(*mtx);
#endif
}

// -- Message Queue ----------------------------------------------------------
void no_mq_create(no_mq_t *mq, unsigned int length, unsigned int msgsz)
{
#if USE_FREERTOS_NATIVE
    QueueHandle_t q = xQueueCreate(length, msgsz);
    configASSERT(q);
    *mq = q;
#else
    *mq = osMessageQueueNew(length, msgsz, NULL);
#endif
}

void no_mq_send(no_mq_t *mq, unsigned int msg)
{
#if USE_FREERTOS_NATIVE
    configASSERT(xQueueSend(*mq, msg, portMAX_DELAY) == pdTRUE);
#else
    osMessageQueuePut(*mq, msg, 0, osWaitForever);
#endif
}

void no_mq_receive(no_mq_t *mq)
{
	unsigned int rcv;
#if USE_FREERTOS_NATIVE
    configASSERT(xQueueReceive(*mq, &rcv, portMAX_DELAY) == pdTRUE);
#else
    osMessageQueueGet(*mq, &rcv, NULL, osWaitForever);
#endif
}

// -- Event Flags ------------------------------------------------------------
void no_event_create(no_event_t *ev)
{
#if USE_FREERTOS_NATIVE
    // FreeRTOS direct notifications could replace event flags
    *ev = NULL;
#else
    osEventFlagsAttr_t a={0};
    *ev = osEventFlagsNew(&a);
#endif
}

void no_event_set(no_event_t *ev)
{
#if !USE_FREERTOS_NATIVE
    osEventFlagsSet(*ev, 0x1);
#endif
}

void no_event_wait(no_event_t *ev)
{
#if !USE_FREERTOS_NATIVE
    osEventFlagsWait(*ev, 0x1, osFlagsWaitAny|osFlagsNoClear, osWaitForever);
#endif
}

// -- ISR Simulation ---------------------------------------------------------
void no_interrupt_register_sgi_handler(no_int_handler_t fn)
{
    NVIC_SetVector((IRQn_Type)EXTI4_IRQn, (uint32_t)fn);
    NVIC_SetPriority(EXTI4_IRQn, 15);
    NVIC_EnableIRQ(EXTI4_IRQn);
}

void no_interrupt_do_sgi(void)
{
    NVIC_SetPendingIRQ(EXTI4_IRQn);
}

void no_interrupt_enable_sgi(void)
{
    // Clear any pending interrupt, then enable it
    NVIC_ClearPendingIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
}


// -- Result Reporting -------------------------------------------------------
void no_single_result_report(char *prefix, int64_t time)
{
    char buf[100];
    snprintf(buf, sizeof(buf), "%s%ld", prefix, (long long)time);
    printf("%s\n", buf);
}

void no_result_report(int64_t max, int64_t min, int64_t average)
{
    uint64_t ns_max;
    int64_t ns_min;
    uint64_t ns_average;
    const int64_t cpu_hz = SystemCoreClock;
    printf("System Core Clock: %lu\n", SystemCoreClock);

    // 1) Print raw cycle counts
    printf("min = %ld cycles, ", (long)min);
    printf("max = %lu cycles, ",(long)max);
    printf("average = %lu cycles\n",(long)average);

    // 2) Convert cycles -> nanoseconds:  ns = (cycles * 1e9) / cpu_hz
    ns_min     = (min * 1000000000UL);
    ns_min     /= cpu_hz;
    ns_max     = (max * 1000000000UL) / cpu_hz;
    ns_average = (average * 1000000000UL) / cpu_hz;

    // 3) Print the converted values
    printf("min = %ld ns, max = %lu ns, average = %lu ns\n",
           (long)ns_min,
           (unsigned long)ns_max,
           (unsigned long)ns_average);
}
