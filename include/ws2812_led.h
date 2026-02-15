#ifndef WS2812_LED_H_
#define WS2812_LED_H_

#include "probe_config.h"
#include <stdbool.h>

#ifdef PROBE_WS2812_LED_PIN

void ws2812_led_init(void);
void ws2812_led_set_usb(bool connected);
void ws2812_led_set_dap_connected(bool connected);
void ws2812_led_set_dap_running(bool running);
void ws2812_led_update(void);

#else

static inline void ws2812_led_init(void) {}
static inline void ws2812_led_set_usb(bool connected) { (void)connected; }
static inline void ws2812_led_set_dap_connected(bool connected) { (void)connected; }
static inline void ws2812_led_set_dap_running(bool running) { (void)running; }
static inline void ws2812_led_update(void) {}

#endif

#endif
