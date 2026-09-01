/* timing_tests.c
 * All ten timing test scenario implementations
 * Author: Sandip Kumar Mourya
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "gpio_markers.h"
#include "timing_tests.h"

LOG_MODULE_REGISTER(timing_tests, LOG_LEVEL_INF);

/* DWT cycle counter access */
#define DWT_CTRL   (*((volatile uint32_t *)0xE0001000))
#define DWT_CYCCNT (*((volatile uint32_t *)0xE0001004))
#define DEM_CR     (*((volatile uint32_t *)0xE000EDFC))
#define DEM_CR_TRCENA (1 << 24)

static void dwt_enable(void)
{
    DEM_CR    |= DEM_CR_TRCENA;
    DWT_CYCCNT = 0;
    DWT_CTRL  |= 1;
}

/* ── Category A: RTOS sleep tests ───────────────────────────── */

static void test_periodic_toggle(uint32_t interval_ms, uint32_t duration_ms)
{
    uint32_t iterations = duration_ms / interval_ms;

    LOG_INF("Periodic toggle: interval=%u ms, iters=%u",
            interval_ms, iterations);

    k_sleep(K_MSEC(1));  /* align to tick boundary */

    marker1_set();

    for (uint32_t i = 0; i < iterations; i++) {
        TFL_MARK();
        marker0_toggle();
        k_sleep(K_MSEC(interval_ms));
    }

    marker0_clear();
    marker1_clear();

    LOG_INF("Periodic toggle done: %u iters", iterations);
}

/* ── Category B: Busy-wait tests ────────────────────────────── */

static void test_busy_wait_accuracy(uint32_t duration_ms)
{
    static const uint32_t delays_us[] =
        {10, 50, 100, 500, 1000, 5000, 10000};

    LOG_INF("Busy-wait accuracy test");
    marker1_set();

    for (int d = 0; d < (int)ARRAY_SIZE(delays_us); d++) {
        uint32_t delay = delays_us[d];
        LOG_INF("  Sub-test: busy_wait(%u us)", delay);

        for (int p = 0; p <= d; p++) {
            marker2_pulse();
            k_busy_wait(100);
        }

        k_sleep(K_MSEC(10));

        for (int i = 0; i < 100; i++) {
            marker0_set();
            k_busy_wait(delay);
            marker0_clear();
            k_busy_wait(delay);
        }

        k_sleep(K_MSEC(50));
    }

    marker1_clear();
    LOG_INF("Busy-wait accuracy done");
}

/* ── Category C: Kernel timer callback tests ─────────────────── */

static struct k_timer timing_timer;
static volatile uint32_t timer_cb_count = 0;

static void timer_cb(struct k_timer *t)
{
    TFL_MARK();
    marker0_toggle();
    timer_cb_count++;
}

static void test_timer_callback_jitter(uint32_t period_ms,
                                       uint32_t duration_ms)
{
    LOG_INF("Timer callback jitter: period=%u ms, duration=%u ms",
            period_ms, duration_ms);

    k_timer_init(&timing_timer, timer_cb, NULL);
    timer_cb_count = 0;

    marker1_set();
    k_timer_start(&timing_timer, K_MSEC(period_ms), K_MSEC(period_ms));
    k_sleep(K_MSEC(duration_ms));
    k_timer_stop(&timing_timer);
    marker0_clear();
    marker1_clear();

    LOG_INF("Timer callback done: %u callbacks", timer_cb_count);
}

/* ── Category D: Thread scheduling tests ────────────────────── */

#define SCHED_STACK_SIZE 512
#define SCHED_PRIORITY   2
#define WORKER_STACK_SIZE 512

static K_THREAD_STACK_DEFINE(sched_stack,    SCHED_STACK_SIZE);
static K_THREAD_STACK_DEFINE(worker1_stack,  WORKER_STACK_SIZE);
static K_THREAD_STACK_DEFINE(worker2_stack,  WORKER_STACK_SIZE);

static struct k_thread sched_td, worker1_td, worker2_td;

static volatile bool sched_running   = false;
static volatile bool workers_running = false;
static volatile uint32_t workload_intensity = 1000;

static void sched_thread(void *p1, void *p2, void *p3)
{
    uint32_t period = (uint32_t)(uintptr_t)p1;
    int64_t  next   = k_uptime_get();

    while (sched_running) {
        next += period;
        TFL_MARK();
        marker0_toggle();
        k_sleep(K_TIMEOUT_ABS_MS(next));
    }

    marker0_clear();
}

static void cpu_worker(void *p1, void *p2, void *p3)
{
    while (workers_running) {
        volatile uint32_t x = 0;
        for (volatile uint32_t i = 0; i < workload_intensity; i++) {
            x += i * i;
        }
        k_yield();
    }
}

static void io_worker(void *p1, void *p2, void *p3)
{
    while (workers_running) {
        marker2_toggle();
        k_sleep(K_MSEC(3));
    }
    marker2_clear();
}

static void test_thread_scheduling_jitter(uint32_t period_ms,
                                          uint32_t duration_ms)
{
    LOG_INF("Thread scheduling jitter: period=%u ms, duration=%u ms",
            period_ms, duration_ms);

    sched_running = true;
    marker1_set();

    k_thread_create(&sched_td, sched_stack, SCHED_STACK_SIZE,
                    sched_thread, (void *)(uintptr_t)period_ms,
                    NULL, NULL, SCHED_PRIORITY, 0, K_NO_WAIT);

    k_sleep(K_MSEC(duration_ms));

    sched_running = false;
    k_thread_join(&sched_td, K_MSEC(500));
    marker1_clear();

    LOG_INF("Thread scheduling jitter done");
}

static void test_multi_thread_interference(uint32_t period_ms,
                                           uint32_t duration_ms)
{
    LOG_INF("Multi-thread interference: period=%u ms", period_ms);

    sched_running   = true;
    workers_running = true;
    workload_intensity = 10000;

    marker1_set();

    k_thread_create(&sched_td, sched_stack, SCHED_STACK_SIZE,
                    sched_thread, (void *)(uintptr_t)period_ms,
                    NULL, NULL, SCHED_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&worker1_td, worker1_stack, WORKER_STACK_SIZE,
                    cpu_worker, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    k_thread_create(&worker2_td, worker2_stack, WORKER_STACK_SIZE,
                    io_worker, NULL, NULL, NULL, SCHED_PRIORITY, 0, K_NO_WAIT);

    k_sleep(K_MSEC(duration_ms));

    sched_running   = false;
    workers_running = false;
    k_thread_join(&sched_td,   K_MSEC(500));
    k_thread_join(&worker1_td, K_MSEC(500));
    k_thread_join(&worker2_td, K_MSEC(500));

    marker1_clear();
    LOG_INF("Multi-thread interference done");
}

/* ── Category E: System-level tests ─────────────────────────── */

static void test_isr_latency(uint32_t duration_ms)
{
    LOG_INF("ISR latency via timer callback: duration=%u ms", duration_ms);
    test_timer_callback_jitter(1, duration_ms);
}

static void test_workload_impact(uint32_t duration_ms)
{
    uint32_t phase = duration_ms / 4;
    LOG_INF("Workload impact: 4 phases x %u ms", phase);

    /* Phase 1: no load */
    LOG_INF("  Phase 1: No load");
    marker3_clear();
    test_timer_callback_jitter(10, phase);
    k_sleep(K_MSEC(100));

    /* Phase 2: light load */
    LOG_INF("  Phase 2: Light load");
    marker3_set();
    workers_running    = true;
    workload_intensity = 1000;
    k_thread_create(&worker1_td, worker1_stack, WORKER_STACK_SIZE,
                    cpu_worker, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
    test_timer_callback_jitter(10, phase);
    k_sleep(K_MSEC(100));

    /* Phase 3: heavy load */
    LOG_INF("  Phase 3: Heavy load");
    workload_intensity = 100000;
    test_timer_callback_jitter(10, phase);
    k_sleep(K_MSEC(100));

    /* Phase 4: extreme load */
    LOG_INF("  Phase 4: Extreme load");
    workload_intensity = 1000000;
    test_timer_callback_jitter(10, phase);

    workers_running = false;
    marker3_clear();
    k_sleep(K_MSEC(100));

    LOG_INF("Workload impact done");
}

/* ── Dispatcher ──────────────────────────────────────────────── */

void run_timing_test(test_id_t test, uint32_t duration_ms)
{
    LOG_INF("=== Starting test %d (duration=%u ms) ===", test, duration_ms);

    switch (test) {
    case TEST_PERIODIC_TOGGLE_1MS:
        test_periodic_toggle(1, duration_ms); break;
    case TEST_PERIODIC_TOGGLE_10MS:
        test_periodic_toggle(10, duration_ms); break;
    case TEST_PERIODIC_TOGGLE_100MS:
        test_periodic_toggle(100, duration_ms); break;
    case TEST_BUSY_WAIT_ACCURACY:
        test_busy_wait_accuracy(duration_ms); break;
    case TEST_SLEEP_ACCURACY:
        test_periodic_toggle(5, duration_ms); break;
    case TEST_TIMER_CALLBACK_JITTER:
        test_timer_callback_jitter(10, duration_ms); break;
    case TEST_THREAD_SCHEDULING_JITTER:
        test_thread_scheduling_jitter(10, duration_ms); break;
    case TEST_MULTI_THREAD_INTERFERENCE:
        test_multi_thread_interference(10, duration_ms); break;
    case TEST_ISR_LATENCY:
        test_isr_latency(duration_ms); break;
    case TEST_WORKLOAD_IMPACT:
        test_workload_impact(duration_ms); break;
    default:
        LOG_ERR("Unknown test id: %d", test); break;
    }

    LOG_INF("=== Test %d complete ===", test);
}
