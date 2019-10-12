/* ubmc_gpio
 *
 * Copyright 2012 Manuel Traut <manut@linutronix.de>
 *
 * LGPL licensed
 */

#ifndef _UBMC_GPIO_H_
#define _UBMC_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _ubmc_gpio_direction {
	GPIO_IN,
    GPIO_OUT,
} ubmc_gpio_direction;

typedef enum _ubmc_gpio_value {
	GPIO_LOW = 0,
    GPIO_HIGH = 1,
} ubmc_gpio_value;

typedef enum _ubmc_gpio_irq_mode {
	GPIO_NONE,
    GPIO_RISING,
    GPIO_FALLING,
    GPIO_BOTH,
} ubmc_gpio_irq_mode;

typedef enum _ubmc_gpio_status {
	GPIO_INVALID = 0, 
    GPIO_VALID = 1,
} ubmc_gpio_status;

typedef struct _ubmc_gpio_pin {
	unsigned int no;
	ubmc_gpio_direction direction;
	ubmc_gpio_irq_mode irq_mode;
	int fd;
	ubmc_gpio_status valid;
} ubmc_gpio_pin;

int ubmc_gpio_open(ubmc_gpio_pin *pin, unsigned int no);
int ubmc_gpio_open_by_name(ubmc_gpio_pin *pin, const char *name);
int ubmc_gpio_open_dir(ubmc_gpio_pin *pin, unsigned int no, ubmc_gpio_direction dir);
int ubmc_gpio_open_by_name_dir(ubmc_gpio_pin *pin, const char *name, ubmc_gpio_direction dir);

int ubmc_gpio_close(ubmc_gpio_pin *pin);
void ubmc_gpio_destroy(void);

int ubmc_gpio_out(ubmc_gpio_pin *pin);
int ubmc_gpio_in(ubmc_gpio_pin *pin);

int ubmc_gpio_set_value(ubmc_gpio_pin *pin, ubmc_gpio_value value);
int ubmc_gpio_get_value(ubmc_gpio_pin *pin, ubmc_gpio_value *value);

int ubmc_gpio_enable_irq(ubmc_gpio_pin *pin, ubmc_gpio_irq_mode m);
int ubmc_gpio_irq_wait(ubmc_gpio_pin *pin, ubmc_gpio_value *value);
int ubmc_gpio_irq_timed_wait(ubmc_gpio_pin *pin, ubmc_gpio_value *value, int timeout_ms);

int ubmc_gpio_get_fd(ubmc_gpio_pin *pin);

#ifdef __cplusplus
}
#endif

#endif
