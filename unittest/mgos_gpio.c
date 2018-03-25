#include "mgos.h"
#include "mgos_gpio.h"

static mgos_gpio_int_handler_f s_handler_cb;
static void *s_handler_cb_arg;

bool mgos_gpio_set_mode(int pin, enum mgos_gpio_mode mode) {
  LOG(LL_INFO, ("Setting pin=%d to mode=%d", pin, mode));
  return true;
}

void mgos_gpio_write(int pin, bool level) {
  LOG(LL_INFO, ("Setting pin=%d to %s", pin, level?"HIGH":"LOW"));
}

bool mgos_gpio_set_button_handler(int pin, enum mgos_gpio_pull_type pull_type, enum mgos_gpio_int_mode int_mode, int debounce_ms, mgos_gpio_int_handler_f cb, void *arg) {
  s_handler_cb = cb;
  s_handler_cb_arg = arg;

  return true;
  (void) debounce_ms;
  (void) int_mode;
  (void) pull_type;
  (void) pin;
}

void mgos_gpio_inject(int pin) {
  if (s_handler_cb)
    s_handler_cb(pin, s_handler_cb_arg);
}

