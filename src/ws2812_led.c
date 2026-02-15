#include "probe_config.h"

#ifdef PROBE_WS2812_LED_PIN

#include <hardware/pio.h>
#include "ws2812_led.pio.h"
#include "ws2812_led.h"

/* Use pio1 to avoid conflict with probe (pio0 SM0) and autobaud (pio0 dynamic) */
#define WS2812_PIO  pio1
#define WS2812_SM   0
#define WS2812_FREQ 800000

/* State flags - written by various threads, read by ws2812_led_update() */
static volatile bool state_usb_connected;
static volatile bool state_dap_connected;
static volatile bool state_dap_running;

static uint ws2812_offset;
static uint32_t last_color;

/* Color definitions (0xRRGGBB) */
#define COLOR_RED       0xFF0000
#define COLOR_BLUE      0x0000FF
#define COLOR_GREEN     0x00FF00
#define COLOR_YELLOW    0xFFAA00

/* Dim to ~6% brightness - full WS2812 is blinding */
#define BRIGHTNESS_SHIFT 4

static inline void put_pixel(uint32_t color_rgb) {
    uint8_t r = ((color_rgb >> 16) & 0xFF) >> BRIGHTNESS_SHIFT;
    uint8_t g = ((color_rgb >> 8) & 0xFF) >> BRIGHTNESS_SHIFT;
    uint8_t b = (color_rgb & 0xFF) >> BRIGHTNESS_SHIFT;
    /* WS2812 expects GRB order, MSB first, in bits [31:8] for 24-bit autopush */
    pio_sm_put_blocking(WS2812_PIO, WS2812_SM,
        ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8));
}

void ws2812_led_init(void) {
    state_usb_connected = false;
    state_dap_connected = false;
    state_dap_running = false;
    last_color = 0xFFFFFFFF; /* Force first update */

    ws2812_offset = pio_add_program(WS2812_PIO, &ws2812_program);
    ws2812_program_init(WS2812_PIO, WS2812_SM, ws2812_offset,
                        PROBE_WS2812_LED_PIN, WS2812_FREQ, false);

    put_pixel(COLOR_RED);
    last_color = COLOR_RED;
}

void ws2812_led_set_usb(bool connected) {
    state_usb_connected = connected;
}

void ws2812_led_set_dap_connected(bool connected) {
    state_dap_connected = connected;
}

void ws2812_led_set_dap_running(bool running) {
    state_dap_running = running;
}

void ws2812_led_update(void) {
    uint32_t color;

    if (state_dap_running)
        color = COLOR_YELLOW;
    else if (state_dap_connected)
        color = COLOR_GREEN;
    else if (state_usb_connected)
        color = COLOR_BLUE;
    else
        color = COLOR_RED;

    if (color != last_color) {
        put_pixel(color);
        last_color = color;
    }
}

#endif
