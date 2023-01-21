#include "uart.h"
#include "i2c.h"

int main(int argc, const char * argv[]) {


    float env_temp = init_i2c("/dev/i2c-1");
    useUart('1');

    return 0;
}
