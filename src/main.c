#include <stdio.h>
#include <stdlib.h>

#include "bsp/board.h"
#include "hardware/pio.h"
// #include "pico/stdlib.h"

#include "tusb.h"
#include "ws2812.pio.h"

#define HID_INTERVAL 1000
#define IS_RGBW true
#define MOUSE_DELTA_MIN 10
#define MOUSE_DELTA_MAX 20

#ifdef PICO_DEFAULT_WS2812_PIN
#define LED_PIN PICO_DEFAULT_WS2812_PIN
#else
#define LED_PIN 16
#endif

enum {
  ITF_KEYBOARD = 0,
  ITF_MOUSE = 1
};

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 500,
  BLINK_SUSPENDED = 1000,
};

static uint32_t blink_interval = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

int main(void)
{
  // stdio_init_all();
  board_init();
  tud_init(BOARD_TUD_RHPORT); // TUD root hub port

  PIO pio = pio0;
  int sm = 0;
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, sm, offset, LED_PIN, 800000, IS_RGBW);

  while (1) {
    tud_task();
    led_blinking_task();
    hid_task();
  }

  return 0;
}

void tud_mount_cb(void)
{
  blink_interval = BLINK_MOUNTED;
}

void tud_umount_cb(void)
{
  blink_interval = BLINK_NOT_MOUNTED;
  return;
}

void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval = BLINK_SUSPENDED;
}

void tud_resume_cb(void)
{
  blink_interval = BLINK_MOUNTED;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) report;
  (void) len;
}

// https://github.com/raspberrypi/pico-examples/blob/master/pio/ws2812/ws2812.c
static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t) (r) << 8) |
         ((uint32_t) (g) << 16) |
         (uint32_t) (b);
}

void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  if (board_millis() - start_ms < blink_interval) return;
  start_ms += blink_interval;

  if (led_state) {
    put_pixel(urgb_u32(0, 0x50, 0));
  } else {
    put_pixel(urgb_u32(0, 0, 0));
  }

  board_led_write(led_state);
  led_state = !led_state;
}

void hid_task(void)
{
  static uint32_t start_ms = 0; // Smaller than get_absolute_time() data

  if (board_millis() - start_ms < HID_INTERVAL) return;
  start_ms += HID_INTERVAL;

  if (tud_suspended()) {
    tud_remote_wakeup();
  }

  // https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid_device.c
  if (tud_hid_n_ready(ITF_KEYBOARD)) {
    static bool has_key = false;

    if (!has_key) {
      uint8_t modifier = 0;
      uint8_t keycode[6] = { 0 };

      // https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
      modifier = rand() % 2 ? KEYBOARD_MODIFIER_LEFTALT : KEYBOARD_MODIFIER_LEFTGUI; // WIN or MAC?
      // keycode[0] = rand() % 0x24 + 4; // HID_KEY_A to HID_KEY_0
      keycode[0] = HID_KEY_TAB;

      tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, modifier, keycode);

      has_key = true;
    } else {
      tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, 0, NULL);

      has_key = false;
    }
  }

  if (tud_hid_n_ready(ITF_MOUSE)) {
    int8_t delta_x = 0;
    int8_t delta_y = 0;

    delta_x = rand() % (1 + MOUSE_DELTA_MAX - MOUSE_DELTA_MIN) + MOUSE_DELTA_MIN;
    delta_y = rand() % (1 + MOUSE_DELTA_MAX - MOUSE_DELTA_MIN) + MOUSE_DELTA_MIN;
    delta_x = rand() % 2 ? delta_x : -delta_x;
    delta_y = rand() % 2 ? delta_y : -delta_y;

    tud_hid_n_mouse_report(ITF_MOUSE, 0, 0x00, delta_x, delta_y, 0, 0);
  }
}
