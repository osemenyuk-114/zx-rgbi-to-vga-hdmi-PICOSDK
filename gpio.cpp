#include "hardware/gpio.h"

#include "gpio.h"

// GPIO Functions //////////////////////////////////////////////////////////////

void pinMode(unsigned int pin, bool out)
{
    gpio_init(pin);
    gpio_set_dir(pin, out);
}

void digitalWrite(unsigned int pin, bool value)
{
    gpio_put(pin, value);
}

int digitalRead(unsigned int pin)
{
    return gpio_get(pin);
}
