#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include "uart.h"
#include "i2c.h"
#include "gpio.h"
#include "view.h"

float temperatura_referencial;
float temperatura_interna;
float temperatura_ambiente = 20;
int estado_sistema;
int estado_funcionamento;
int modo_controle = 0;
int valor_pwm = 0;
unsigned inicio_curva;

FILE *arquivo_log;

void zera_sistema()
{
    useUart('6', 0);
    estado_sistema = 0;
    useUart('7', 0);
    useUart('8', 0);
    estado_funcionamento = 0;

    float aux = useUart('1', 0);
    if (aux > 0)
    {
        temperatura_interna = aux;
    }

    valor_pwm = 0;
    set_pwm_value(valor_pwm);
    useUart('4', valor_pwm);
}

void finish(int sig)
{
    signal(sig, SIG_IGN);

    zera_sistema();
    fclose(arquivo_log);

    exit(0);
}

void init()
{
    arquivo_log = fopen("log.csv", "w+");
    fprintf(arquivo_log, "Data/Hora, Temp Interna, Temp Externa, Temp Referencial, PWM\n");

    int opcao_temperatura = pede_opcao_temperatura();

    if (opcao_temperatura == 2)
    {
        modo_controle = 3;
        temperatura_referencial = pede_temperatura();
        useUart('5', temperatura_referencial);
    }
    else
    {
        float aux = useUart('2', 0);
        if (aux)
        {
            temperatura_referencial = aux;
        }
    }

    int opcao_PID = pede_opcao_PID();

    double Kp = 30;
    double Ki = 0.2;
    double Kd = 400;

    if (opcao_PID == 2)
    {
        Kp = pede_Kp();
        Ki = pede_Ki();
        Kd = pede_Kd();
    }

    gpio_init(Kp, Ki, Kd);

    zera_sistema();
}

void escreve_log(int pwm)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(arquivo_log, "%02d/%02d/%d %02d:%02d:%02d, %.2f, %.2f, %.2f, %d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, temperatura_interna, temperatura_ambiente, temperatura_referencial, pwm);
}

void controla_curva()
{

    unsigned timestamp_atual = (unsigned)time(NULL);
    int tempo_atual = (int)timestamp_atual - inicio_curva;
    float temperatura_ref = -1;

    FILE *arquivo_curva = fopen("curva_reflow.csv", "r");

    char linha[30];
    fgets(linha, 30, arquivo_curva);

    while (fgets(linha, 22, arquivo_curva) != NULL)
    {
        int index = 0;
        while (linha[++index] != ',')
            ;

        char tempo_string[index];

        for (int i = 0; i < index; i++)
        {
            tempo_string[i] = linha[i];
        }

        int tempo = atoi(tempo_string);

        if (tempo_atual > tempo)
        {
            char temperatura_string[2];

            while (linha[++index] == ' ')
                ;

            temperatura_string[0] = linha[index++];
            temperatura_string[1] = linha[index++];

            temperatura_ref = atof(temperatura_string);
        }
        else
        {
            break;
        }
    }

    float temp_aux = useUart('1', 0);
    if (temp_aux > 0)
    {
        temperatura_interna = temp_aux;
    }

    printf("temperatura_ref: %lf\n", temperatura_ref);
    useUart('5', temperatura_ref);

    temperatura_referencial = temperatura_ref;
}

void controla_temperatura()
{
    if (estado_sistema)
    {
        float temp_aux = useUart('1', 0);
        if (temp_aux > 0)
        {
            temperatura_interna = temp_aux;
        }

        temp_aux = init_i2c("/dev/i2c-1");
        if (temp_aux > 0)
        {
            temperatura_ambiente = temp_aux;
        useUart('9', temperatura_ambiente);
        }

        if (estado_funcionamento)
        {
            if (modo_controle == 0)
            {
                temp_aux = useUart('2', 0);
                if (temp_aux > 0)
                {
                    temperatura_referencial = temp_aux;
                }
            }
            else
            {
                controla_curva();
            }

            valor_pwm = control_pwm(temperatura_interna, temperatura_referencial);
            useUart('4', valor_pwm);

            escreve_log(valor_pwm);
        }
    }
}

void loop()
{
    while (1)
    {
        float commandF = useUart('3', 0);
        int command = (int)commandF;

        switch (command)
        {
        case 0xA1:
            if (useUart('6', 1))
            {
                estado_sistema = 1;
            }
            break;

        case 0xA2:
            if (useUart('6', 0))
            {
                estado_sistema = 0;
                zera_sistema();
            }
            break;

        case 0xA3:
            if (estado_sistema && useUart('8', 1))
            {
                estado_funcionamento = 1;
            }
            break;

        case 0xA4:
            if (useUart('8', 0))
            {
                estado_funcionamento = 0;
                valor_pwm = 0;
                set_pwm_value(valor_pwm);
                useUart('4', valor_pwm);
            }
            break;

        case 0xA5:
            if (modo_controle == 0)
            {

                useUart('7', 1);
                modo_controle = 1;
                inicio_curva = (unsigned)time(NULL);
            }
            else
            {
                useUart('7', 0);
                modo_controle = 0;
            }
        }

        controla_temperatura();

        usleep(500000);
    }
}

int main(int argc, const char *argv[])
{
    signal(SIGINT, finish);
    init();
    loop();

    return 0;
}
