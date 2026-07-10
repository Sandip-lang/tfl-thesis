/* timing_tests.h
 * Timing test scenario declarations
 * Author: Sandip Kumar Mourya
 */

#ifndef TIMING_TESTS_H
#define TIMING_TESTS_H

typedef enum {
    TEST_PERIODIC_TOGGLE_1MS   = 0,
    TEST_PERIODIC_TOGGLE_10MS  = 1,
    TEST_PERIODIC_TOGGLE_100MS = 2,
    TEST_BUSY_WAIT_ACCURACY    = 3,
    TEST_SLEEP_ACCURACY        = 4,
    TEST_TIMER_CALLBACK_JITTER = 5,
    TEST_THREAD_SCHEDULING_JITTER  = 6,
    TEST_MULTI_THREAD_INTERFERENCE = 7,
    TEST_ISR_LATENCY           = 8,
    TEST_WORKLOAD_IMPACT       = 9,
} test_id_t;

/* Run a single test scenario */
void run_timing_test(test_id_t test, uint32_t duration_ms);

#endif /* TIMING_TESTS_H */
