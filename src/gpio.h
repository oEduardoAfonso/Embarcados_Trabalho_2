void gpio_init(double Kp_, double Ki_, double Kd_);

int control_pwm(float temp_interna, float temp_referencia);

void set_pwm_value(int pwm_value);
