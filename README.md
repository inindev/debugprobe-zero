# Debugprobe on RP2040-Zero

Fork of the [Raspberry Pi Debugprobe](https://github.com/raspberrypi/debugprobe) firmware adapted for the [Waveshare RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero).

The RP2040-Zero is a tiny board with the same RP2040 chip as the Pico, but in a much smaller form factor — ideal for a permanent debug probe.

## Pin Assignment

| Function | GPIO | Notes |
|----------|------|-------|
| SWCLK | GP10 | SWD clock |
| SWDIO | GP11 | SWD data |
| GND | GP12 | Driven low in software as a ground pin |
| UART TX | GP4 | Target serial RX |
| UART RX | GP5 | Target serial TX |
| Reset | GP1 | Target nRESET (active low, optional) |
| WS2812 LED | GP16 | On-board RGB status LED |

GP10, GP11, and GP12 are adjacent on the board, giving a convenient 3-pin SWD + GND cluster.

## Building

Requires Pico SDK v2.0.0+ and the FreeRTOS submodule.

```
git submodule update --init --recursive --depth 1
mkdir build
cd build
cmake -DDEBUG_ON_PICO=ON ..
make -j
```

This produces `debugprobe-zero.uf2`. Flash it by holding BOOTSEL on the RP2040-Zero and dragging the file to the USB mass storage drive.

## AutoBaud

To enable automatic UART baud rate detection, set the USB CDC port to the magic baud rate:

```
9728 (0x2600)
```

Setting any other baud rate disables AutoBaud and uses the requested rate.

## WS2812 RGB LED Status

The RP2040-Zero has no regular LED (the Pico's GPIO 25 LED is absent), but it does have a WS2812 addressable RGB LED on GP16. This firmware drives it as a probe status indicator using PIO1.

### Color Scheme

| State | Color | Meaning |
|-------|-------|---------|
| USB not connected / suspended | Red | Probe is not ready |
| USB connected, idle | Blue | Ready, waiting for a debugger to connect |
| DAP debugger connected | Green | Debugger (e.g. OpenOCD) is attached |
| Target running | Yellow/Amber | Target MCU is executing code |

Priority (highest to lowest): Target running > DAP connected > USB connected > Disconnected.

Brightness is dimmed to approximately 12% to avoid being blinding at close range.

### Architecture

Multiple FreeRTOS threads set lightweight volatile state flags:

- **USB thread** polls `tud_ready()` each iteration and sets the USB connected state. The mount/unmount/suspend/resume callbacks also update it immediately.
- **DAP thread** receives `DAP_HostStatus` commands from the debugger, which trigger `LED_CONNECTED_OUT` and `LED_RUNNING_OUT` in DAP_config.h. These set the DAP connected and target running flags.

A single `ws2812_led_update()` function, called from the USB thread, resolves the priority and sends a color to the LED via PIO1. The LED is only written when the color actually changes.

The WS2812 PIO program runs on **PIO1 SM0**, avoiding any conflict with the probe SWD interface (PIO0 SM0) and autobaud detection (PIO0, dynamically claimed).

### Conditional Compilation

The entire WS2812 feature is gated on `#define PROBE_WS2812_LED_PIN` in the board config header. When not defined, all WS2812 functions compile to no-ops via static inline stubs. The standard Debug Probe and Pico board configs are unaffected.
