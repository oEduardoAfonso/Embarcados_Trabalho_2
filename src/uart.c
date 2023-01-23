#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include "crc16.h"

const unsigned char GET_INTERN_TEMP[] = {0x01, 0x23, 0xC1, 2, 3, 0, 7, -1, -1};
const unsigned char GET_REF_TEMP[] = {0x01, 0x23, 0xC2, 2, 3, 0, 7, -1, -1};
const unsigned char GET_USER_INPUT[] = {0x01, 0x23, 0xC3, 2, 3, 0, 7, -1, -1};
const unsigned char SEND_CONTROL_SIGNAL[] = {0x01, 0x23, 0xD1, 2, 3, 0, 7, -1, -1, -1, -1, -1, -1};
const unsigned char SEND_REF_SIGNAL[] = {0x01, 0x23, 0xD2, 2, 3, 0, 7, -1, -1, -1, -1, -1, -1};
const unsigned char SEND_SYSTEM_STATUS[] = {0x01, 0x23, 0xD3, 2, 3, 0, 7, -1, -1, -1};
const unsigned char SEND_TEMP_MODE[] = {0x01, 0x23, 0xD4, 2, 3, 0, 7, -1, -1, -1};
const unsigned char SEND_FUNC_STATUS[] = {0x01, 0x23, 0xD5, 2, 3, 0, 7, -1, -1, -1};
const unsigned char SEND_ENV_TEMP[] = {0x01, 0x23, 0xD6, 2, 3, 0, 7, -1, -1, -1, -1, -1, -1};

const float RESPONSE[] = {-1, -1, -1, -1};

float handle_response(char command, unsigned char *rx_buffer, int size)
{
    rx_buffer[size] = '\0';
    printf("Bytes lidos [%d]: ", size);

    for (int i = 0; i < 3; i++)
    {
        printf("0x%X ", rx_buffer[i]);
    }

    printf("\n");

    if (command == '1' || command == '2')
    {
        unsigned char fBuffer[4] = {rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6]};
        float f;

        memcpy(&f, &fBuffer, 4);

        return f;
    }
    else if (command == '3')
    {
        unsigned char fBuffer[4] = {rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6]};
        int i;

        memcpy(&i, &fBuffer, 4);

        float f = i;

        return f;
    }

    return 1;
}

int isCfcVAlid(unsigned char *command, int size)
{
    short calculated_crc = calcula_CRC(command, size - 2);

    short crc;
    memcpy(&crc, &command[size - 2], 2);

    if (crc == calculated_crc)
    {
        printf("CFC Validado: [%hi]\n", crc);
        return 1;
    }

    return 0;
}

float getUart(int uart0_filestream, char command)
{
    int rx_length = 0;
    int count = 0;

    while (rx_length < 4 && count != 3)
    {
        usleep(100000);
        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[10];
        rx_length = read(uart0_filestream, (void *)rx_buffer, 9); // Filestream, buffer to store in, number of bytes to read (max)

        if (rx_length < 0)
        {
            printf("Erro na leitura.\n"); // An error occured (will occur if there are no bytes)
        }
        else if (rx_length > 0)
        {
            if (isCfcVAlid(rx_buffer, rx_length))
            {
                return handle_response(command, rx_buffer, rx_length);
            }
        }

        count++;
    }

    if (rx_length == 0)
        {
            printf("Nenhum dado disponível.\n"); // No data waiting
        }

    return -1;
}

void sendUart(int uart0_filestream, char command, float arg)
{
    int size = 0;

    if (command == '1' || command == '2' || command == '3')
    {
        size = 7;
    }
    else if (command == '4' || command == '5' || command == '9')
    {
        size = 11;
    }
    else
    {
        size = 8;
    }

    unsigned char tx_buffer[size + 2];
    int arg_i = -1;

    switch (command)
    {
    case '1':
        memcpy(&tx_buffer, GET_INTERN_TEMP, size);
        break;
    case '2':
        memcpy(&tx_buffer, GET_REF_TEMP, size);
        break;
    case '3':
        memcpy(&tx_buffer, GET_USER_INPUT, size);
        break;
    case '4':
        memcpy(&tx_buffer, SEND_CONTROL_SIGNAL, size);
        arg_i = (int)arg;
        memcpy(&tx_buffer[size - 4], &arg_i, 4);
        break;
    case '5':
        memcpy(&tx_buffer, SEND_REF_SIGNAL, size);
        memcpy(&tx_buffer[size - 4], &arg, 4);
        break;
    case '6':
        memcpy(&tx_buffer, SEND_SYSTEM_STATUS, size);
        tx_buffer[size - 1] = arg;
        break;
    case '7':
        memcpy(&tx_buffer, SEND_TEMP_MODE, size);
        tx_buffer[size - 1] = arg;
        break;
    case '8':
        memcpy(&tx_buffer, SEND_FUNC_STATUS, size);
        tx_buffer[size - 1] = arg;
        break;
    case '9':
        memcpy(&tx_buffer, SEND_ENV_TEMP, size);
        memcpy(&tx_buffer[size - 4], &arg, 4);
    }

    short crc = calcula_CRC(tx_buffer, size);

    memcpy(&tx_buffer[size], &crc, 2);

    int count = write(uart0_filestream, tx_buffer, sizeof(tx_buffer));

    printf("Bytes enviados [%d]: %X\n", count, tx_buffer[2]);
}

float useUart(char command, float arg)
{
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY); // Open in non blocking read/write mode

    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD; //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    sendUart(uart0_filestream, command, arg);

    float result = getUart(uart0_filestream, command);

    close(uart0_filestream);

    printf("\n");

    return result;
}
