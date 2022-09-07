# RTOSBench for Zephyr

This repository contains a Zephyr application that executes benchmarks from
[RTOSBench](https://github.com/gchamp20/RTOSBench).

## Getting Started

First set up your Zephyr working environment according to the instructions
provided in Zephyr's [Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html). Then
you can clone the project using Zephyr's metatool `west`. The following shell
commands initialize the project in a workspace directory named `my-workspace`:

```shell
west init -m https://github.com/methodpark/zephyr-rtosbench my-workspace
cd my-workspace
west update
```

## Building and Running

The test cases can be build by running `west build -b $BOARD -s app`. Here
`$BOARD` has to be replaced with the target board. After building, the
respective test case can be executed on hardware by running `west flash` or
`west build -t run`.  Results can be read by attaching to the board's serial
output (e.g. `cat /dev/ttyACM0`).

## Changing the Test Case

By default, the test case located under
`app/lib/rtosbench/tests/context-switch/round_robin.c` is executed. This can be
changed by providing the build command with another value for the variable
`RTOS_BENCH_TEST_CASE`. In order to execute the test case under
`app/lib/rtosbench/tests/mutex/mutex.c`, for example, define
`RTOS_BENCH_TEST_CASE=mutex/mutex` like this:

```shell
west build -b $BOARD -s app -- -DRTOS_BENCH_TEST_CASE=mutex/mutex
```

# Changing the Test Configuration

RTOSBench outputs average, minimum and maximum measured values by default. It is
also possible to retrieve the result of each iterations instead. The west
project exposes this functionality via the CMake parameter `VERBOSE_RESULTS`. It
can be enabled during the build process in the following way:

```shell
west build -b $BOARD -s app -- -DVERBOSE_RESULTS=1
```

Similarily, it is also possible to configure the benchmark's number of
iterations, that is, how many times a test is executed. This is controlled via
the CMake variable `ITERATIONS`. To change it run `west build` like this:

```shell
west build -b $BOARD -s app -- -DITERATIONS=10000
```

The example above runs a test 10000 times.

# Tracing

Zephyr offers builtin support for tracing in various formats. The repository
contains two configuration files that activate all necessary KConfig options for
either SEGGER SystemView or via UART using the Common Tracing Format (CTF).

The benchmark code automatically inserts a delay of 5 seconds before the
benchmark actually starts. This way you should be able to start capturing
tracing data before the test case is executed.

You can build a test case with tracing support for SEGGER SystemView by calling
west in the following way:

```shell
west build -b $BOARD -s app -- -DOVERLAY_CONFIG='prj_sysview.conf'
```

If your board does not have support for SEGGER SystemView, you can trace using
UART with the following command line:

```shell
west build -b $BOARD -s app -- -DOVERLAY_CONFIG='prj_uart_ctf.conf'
```

Since the tracing data is transmitted via the serial console, this configuration
suppresses any console output from the benchmark. In order actually capture the
tracing output, you have to execute the capture script provided by Zephyr under
`zephyr/scripts/trace_capture_uart.py`. A typical command line would look like
this:

```shell
./scripts/tracing/trace_capture_uart.py -d /dev/ttyACM0 -b 115200 -o output_file
```

Follow the instructions from [Zephyr's
Documentation](https://docs.zephyrproject.org/latest/guides/debug_tools/tracing/index.html#using-tracing) to visualise the data in e.g. TraceCompass.
