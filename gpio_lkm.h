#ifndef GPIO_LKM_H
#define GPIO_LKM_H

#define NUM_COM 4

enum command {
    set_out = 1,
    set_in = 2,
    set_low = 3,
    set_high = 4,
    na = NUM_COM+1
};

extern bool gpio_unavailable;
extern int perform_gpio_command(unsigned int gpio, enum command command);
extern int read_gpio_value(unsigned int gpio);

extern int gpio_lkm_free_gpio(unsigned int gpio);

#endif
