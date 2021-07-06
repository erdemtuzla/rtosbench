cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app LANGUAGES C VERSION 0.0.1)

target_include_directories(app PRIVATE src/include lib/rtosbench/include)

set(RTOS_BENCH_TEST_CASE context-switch/round_robin CACHE STRING "Set the
RTOSBench test case to execute")
set(ITERATIONS 1000 CACHE STRING "Set the
number of test case iterations")

# Make sure that we are using cooperative threads for the
# cooperative context switch test scenario.
if (RTOS_BENCH_TEST_CASE STREQUAL context-switch/round_robin)
  add_compile_definitions(BASE_PRIO=-10)
endif()

if (DEFINED VERBOSE_RESULTS)
  add_compile_definitions(NO_VERBOSE_RESULTS=${VERBOSE_RESULTS})
endif()

add_compile_definitions(NB_ITER=${ITERATIONS})

target_sources(app PRIVATE src/zephyr_porting_layer.c src/nvic.c
  "lib/rtosbench/tests/${RTOS_BENCH_TEST_CASE}.c")
