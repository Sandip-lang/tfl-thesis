/* gpio_markers.h
 * GPIO timing marker interface
 * Author: Sandip Kumar Mourya
 */

#ifndef GPIO_MARKERS_H
#define GPIO_MARKERS_H

#include <zephyr/kernel.h>

/* TFL Deadline Monitor mark register.
 * Writing any value signals a task wakeup event to Renode.
 * No-op on real hardware (unmapped address, MPU disabled). */
#define TFL_MONITOR_MARK_ADDR  0x50000000U
#define TFL_MARK() \
    (*((volatile uint32_t *)TFL_MONITOR_MARK_ADDR) = 1U)

/* Initialise all four GPIO timing markers.
 * Must be called before any marker functions. */
int markers_init(void);

/* Marker 0 — primary timing signal */
void marker0_set(void);
void marker0_clear(void);
void marker0_toggle(void);
void marker0_pulse(void);

/* Marker 1 — test envelope (HIGH = test running) */
void marker1_set(void);
void marker1_clear(void);
void marker1_toggle(void);
void marker1_pulse(void);

/* Marker 2 — sub-test index encoding */
void marker2_set(void);
void marker2_clear(void);
void marker2_toggle(void);
void marker2_pulse(void);

/* Marker 3 — load state indicator */
void marker3_set(void);
void marker3_clear(void);
void marker3_toggle(void);
void marker3_pulse(void);

#endif /* GPIO_MARKERS_H */
