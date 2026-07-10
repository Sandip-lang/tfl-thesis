/* gpio_markers.c
 * GPIO timing marker implementation
 * Author: Sandip Kumar Mourya
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "gpio_markers.h"

LOG_MODULE_REGISTER(gpio_markers, LOG_LEVEL_INF);

#define MARKER0_NODE DT_ALIAS(marker0)
#define MARKER1_NODE DT_ALIAS(marker1)
#define MARKER2_NODE DT_ALIAS(marker2)
#define MARKER3_NODE DT_ALIAS(marker3)

static const struct gpio_dt_spec m0 = GPIO_DT_SPEC_GET(MARKER0_NODE, gpios);
static const struct gpio_dt_spec m1 = GPIO_DT_SPEC_GET(MARKER1_NODE, gpios);
static const struct gpio_dt_spec m2 = GPIO_DT_SPEC_GET(MARKER2_NODE, gpios);
static const struct gpio_dt_spec m3 = GPIO_DT_SPEC_GET(MARKER3_NODE, gpios);

int markers_init(void)
{
    int ret;

    if (!gpio_is_ready_dt(&m0) || !gpio_is_ready_dt(&m1) ||
        !gpio_is_ready_dt(&m2) || !gpio_is_ready_dt(&m3)) {
        LOG_ERR("GPIO devices not ready");
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&m0, GPIO_OUTPUT_INACTIVE); if (ret) return ret;
    ret = gpio_pin_configure_dt(&m1, GPIO_OUTPUT_INACTIVE); if (ret) return ret;
    ret = gpio_pin_configure_dt(&m2, GPIO_OUTPUT_INACTIVE); if (ret) return ret;
    ret = gpio_pin_configure_dt(&m3, GPIO_OUTPUT_INACTIVE); if (ret) return ret;

    LOG_INF("Timing markers ready on PA0, PA1, PA4, PA8");
    return 0;
}

void marker0_set(void)    { gpio_pin_set_dt(&m0, 1); }
void marker0_clear(void)  { gpio_pin_set_dt(&m0, 0); }
void marker0_toggle(void) { gpio_pin_toggle_dt(&m0); }
void marker0_pulse(void)  { gpio_pin_set_dt(&m0, 1); gpio_pin_set_dt(&m0, 0); }

void marker1_set(void)    { gpio_pin_set_dt(&m1, 1); }
void marker1_clear(void)  { gpio_pin_set_dt(&m1, 0); }
void marker1_toggle(void) { gpio_pin_toggle_dt(&m1); }
void marker1_pulse(void)  { gpio_pin_set_dt(&m1, 1); gpio_pin_set_dt(&m1, 0); }

void marker2_set(void)    { gpio_pin_set_dt(&m2, 1); }
void marker2_clear(void)  { gpio_pin_set_dt(&m2, 0); }
void marker2_toggle(void) { gpio_pin_toggle_dt(&m2); }
void marker2_pulse(void)  { gpio_pin_set_dt(&m2, 1); gpio_pin_set_dt(&m2, 0); }

void marker3_set(void)    { gpio_pin_set_dt(&m3, 1); }
void marker3_clear(void)  { gpio_pin_set_dt(&m3, 0); }
void marker3_toggle(void) { gpio_pin_toggle_dt(&m3); }
void marker3_pulse(void)  { gpio_pin_set_dt(&m3, 1); gpio_pin_set_dt(&m3, 0); }
