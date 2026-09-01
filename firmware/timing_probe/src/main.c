/* main.c
 * Temporal accuracy test firmware entry point
 * Author: Sandip Kumar Mourya
 * Board: NUCLEO-F103RB (STM32F103RB, Cortex-M3 @ 72 MHz)
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "gpio_markers.h"
#include "timing_tests.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    int ret;

    LOG_INF("==============================================");
    LOG_INF("Temporal Accuracy Test Firmware v1.0");
    LOG_INF("Board: %s", CONFIG_BOARD);
    LOG_INF("SYSCLK: %u Hz", sys_clock_hw_cycles_per_sec());
    LOG_INF("Tick rate: %u Hz", CONFIG_SYS_CLOCK_TICKS_PER_SEC);
    LOG_INF("==============================================");

    ret = markers_init();
    if (ret < 0) {
        LOG_ERR("Marker init failed: %d", ret);
        return ret;
    }

    /* Self-test: flash all markers 3 times to confirm connections */
    LOG_INF("Marker self-test...");
    for (int i = 0; i < 3; i++) {
        marker0_set(); marker1_set(); marker2_set(); marker3_set();
        k_sleep(K_MSEC(50));
        marker0_clear(); marker1_clear(); marker2_clear(); marker3_clear();
        k_sleep(K_MSEC(50));
    }
    LOG_INF("Self-test complete");

    /* Settling delay before test sequence */
    k_sleep(K_MSEC(500));

    LOG_INF("Beginning test sequence...");

    run_timing_test(TEST_PERIODIC_TOGGLE_1MS,         5000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_PERIODIC_TOGGLE_10MS,        5000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_PERIODIC_TOGGLE_100MS,       5000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_BUSY_WAIT_ACCURACY,         10000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_SLEEP_ACCURACY,             10000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_TIMER_CALLBACK_JITTER,       5000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_THREAD_SCHEDULING_JITTER,    5000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_MULTI_THREAD_INTERFERENCE,   5000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_ISR_LATENCY,                 5000);
    k_sleep(K_MSEC(1000));

    run_timing_test(TEST_WORKLOAD_IMPACT,            20000);

    LOG_INF("=== ALL TESTS COMPLETE ===");

    while (1) {
        k_sleep(K_SECONDS(10));
    }

    return 0;
}
