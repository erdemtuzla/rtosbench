# FreeRTOS Port

These FreeRTOS port files can be used to port tests under `tests` folder.

They have been put in another folder because they are updated to be used for STM32CubeIDE projects.

## Getting Started

1- Simply, after you create a project in STM32CubeIDE, put all .c files under `Src` directory, and put all .h files under `Inc` directory.

2- In your `main()` function, there should be `osKernelInitialize();` and `osKernelStart();` lines. Comment them out.
If there are any lines for creating a thread, then remove or comment out them too. `no_initialize_test()` function in `porting_layer.c` file already implements required kernel initialize and start functions.

3- Add these lines into your main function, right above infinite loop

```c
#if ACTIVATE_CONTEXT_SWITCH_TEST
  no_initialize_test(round_robin_stress_initialize_test);
#endif

#if ACTIVATE_INTERRUPT_TEST
  no_initialize_test(interrupt_initialize_test);
#endif

#if ACTIVATE_MQ_TEST
  no_initialize_test(mq_initialize_test);
#endif

#if ACTIVATE_MUTEX_TEST
  no_initialize_test(mutex_initialize_test);
#endif

#if ACTIVATE_SEMAPHORE_TEST
  no_initialize_test(sem_initialize_test);
#endif

```

4- After doing previous steps, you can configure benchmark options from `config.h`. `ACTIVATE_*_TEST` defines which test will be activated. Only activate 1 test at a time. `USE_FREERTOS_NATIVE` offers a capability to switch between CMSIS-V2 and native FreeRTOS API. `NB_ITER` defines how many times a test will be run.


- I was using NUCLEO_F401RE board when I was working with this repository, and it needed another implementation in order to use its cycle count register. You might need to add similar lines of codes to your project to be able to get results. It will probably change from board to board, so research how it can be done for your hardware. If you are using the same board, just put these lines at the top of the main function:

```c
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
```

- If you want to see the results from Serial Wire Viewer (SWV) of STM32CubeIDE, then follow the steps in this resource: [STM wiki](https://wiki.stmicroelectronics.cn/stm32mpu/wiki/How_to_debug_with_Serial_Wire_Viewer_tracing_on_STM32MP15). Also add this function right before your main function:

```c
int _write(int file, char *ptr, int len)
{
	int i=0;
	for(i=0 ; i<len ; i++)
	ITM_SendChar((*ptr++));
	return len;
}
```