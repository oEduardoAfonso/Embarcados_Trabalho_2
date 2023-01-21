#include <unistd.h>

#include "uart.h"
#include "i2c.h"
#include "gpio.h"

int main(int argc, const char *argv[])
{
    gpio_init();

    // float env_temp = init_i2c("/dev/i2c-1");
    // useUart('1');
    // control_pwm(10.9, 11);

    return 0;
}
