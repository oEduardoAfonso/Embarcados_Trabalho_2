#include <stdio.h>

int valida_opcao(int opcao)
{
    if (opcao == 1 || opcao == 2)
    {
        return 1;
    }

    return 0;
}

void limpa_terminal()
{
    for (int a = 0; a < 50; a++)
    {
        printf("\n");
    }
}

int pede_opcao_temperatura()
{
    limpa_terminal();

    printf("Como deseja controlar a temperatura de referencia?\n");
    printf("[1] - Dashboard\n");
    printf("[2] - Terminal (Modo DEBUG)\n");

    int opcao;

    scanf("%d", &opcao);

    if (!valida_opcao(opcao))
    {
        return pede_opcao_temperatura();
    }

    return opcao;
}

int pede_opcao_PID()
{
    limpa_terminal();

    printf("Como deseja definir os valores para o controle PID?\n");
    printf("[1] - Valores padrão (Kp: 30, Ki: 0.2, Kd: 400)\n");
    printf("[2] - Manualmente\n");

    int opcao;

    scanf("%d", &opcao);

    if (!valida_opcao(opcao))
    {
        return pede_opcao_PID();
    }

    return opcao;
}

float pede_temperatura()
{
    limpa_terminal();

    printf("Informe o valor para a temperatura referencial:\n");

    float valor;

    scanf("%f", &valor);

    return valor;
}

double pede_Kp()
{
    limpa_terminal();

    printf("Informe o valor para o Kp:\n");

    double valor;

    scanf("%lf", &valor);

    return valor;
}

double pede_Ki()
{
    limpa_terminal();

    printf("Informe o valor para o Ki:\n");

    double valor;

    scanf("%lf", &valor);

    return valor;
}

double pede_Kd()
{
    limpa_terminal();

    printf("Informe o valor para o Kd:\n");

    double valor;

    scanf("%lf", &valor);

    return valor;
}
