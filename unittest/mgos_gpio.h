#ifndef __MGOS_GPIO_H
#define __MGOS_GPIO_H

#include "mgos.h"

enum mgos_gpio_mode {
  MGOS_GPIO_MODE_INPUT = 0, /* input mode */
  MGOS_GPIO_MODE_OUTPUT = 1 /* output mode */
};

enum mgos_gpio_pull_type {
  MGOS_GPIO_PULL_NONE = 0,
  MGOS_GPIO_PULL_UP = 1,  /* pin is pilled to the high voltage */
  MGOS_GPIO_PULL_DOWN = 2 /* pin is pulled to the low voltage */
};

enum mgos_gpio_int_mode {
  MGOS_GPIO_INT_NONE = 0,
  MGOS_GPIO_INT_EDGE_POS = 1, /* positive edge */
  MGOS_GPIO_INT_EDGE_NEG = 2, /* negative edge */
  MGOS_GPIO_INT_EDGE_ANY = 3, /* any edge - positive or negative */
  MGOS_GPIO_INT_LEVEL_HI = 4, /* high voltage level */
  MGOS_GPIO_INT_LEVEL_LO = 5  /* low voltage level */
};

typedef void (*mgos_gpio_int_handler_f)(int pin, void *arg);

bool mgos_gpio_set_mode(int pin, enum mgos_gpio_mode mode);
void mgos_gpio_write(int pin, bool level);

bool mgos_gpio_set_button_handler(int pin, enum mgos_gpio_pull_type pull_type,
                                  enum mgos_gpio_int_mode int_mode,
                                  int debounce_ms, mgos_gpio_int_handler_f cb,
                                  void *arg);

void mgos_gpio_inject(int pin);


#endif // __MGOS_GPIO_H
