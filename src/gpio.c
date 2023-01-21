#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

#include "pid.h"

#define PIN_RES 4
#define PIN_FAN 5

void gpio_init()
{
    wiringPiSetup();

    pinMode(PIN_RES, OUTPUT);
    pinMode(PIN_FAN, OUTPUT);

    softPwmCreate(PIN_RES, 0, 100);
    softPwmCreate(PIN_FAN, 0, 100);
}

void set_pwm_value(int pwm_value)
{
    if (pwm_value >= 0)
    {
        softPwmWrite(PIN_FAN, 0);
        softPwmWrite(PIN_RES, pwm_value);
    }
    else
    {
        pwm_value *= -1;

        if (pwm_value < 40)
        {
            pwm_value = 40;
        }

        softPwmWrite(PIN_FAN, pwm_value);
        softPwmWrite(PIN_RES, 0);
    }
}

void control_pwm(float temp_interna, float temp_referencia)
{
    pid_atualiza_referencia(temp_referencia);
    int pwm_value = (int)pid_controle(temp_interna);
    printf("int: %f, ref: %f, pwm: %d\n", temp_interna, temp_referencia, pwm_value);
    set_pwm_value(pwm_value);
}
