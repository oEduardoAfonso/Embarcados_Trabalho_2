#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include "crc16.h"

const unsigned char GET_INTERN_TEMP [] = {0x01, 0x23, 0xC1, 2, 3, 0, 7, -1, -1};
const unsigned char GET_REF_TEMP [] = {0x01, 0x23, 0xC2, 2, 3, 0, 7, -1, -1};
const unsigned char GET_USER_INPUT [] = {0x01, 0x23, 0xC3, 2, 3, 0, 7, -1, -1};
const unsigned char SEND_CONTROL_SIGNAL [] = {0x01, 0x23, 0xD1, 2, 3, 0, 7, -1, -1, -1, -1, -1, -1};
const unsigned char SEND_REF_SIGNAL [] = {0x01, 0x23, 0xD2, 2, 3, 0, 7, -1, -1, -1, -1, -1, -1};
const unsigned char SEND_SYSTEM_STATUS [] = {0x01, 0x23, 0xD3, 2, 3, 0, 7, -1, -1, -1};
const unsigned char SEND_TEMP_MODE [] = {0x01, 0x23, 0xD4, 2, 3, 0, 7, -1, -1, -1};
const unsigned char SEND_FUNC_STATUS [] = {0x01, 0x23, 0xD5, 2, 3, 0, 7, -1, -1, -1};
const unsigned char SEND_ENV_TEMP [] = {0x01, 0x23, 0xD6, 2, 3, 0, 7, -1, -1, -1, -1, -1, -1};


void printRx(unsigned char *rx_buffer, int size) {
    rx_buffer[size] = '\0';
    printf("Bytes lidos [%d]: ", size);

    for(int i = 0; i < 3; i++){
        printf("0x%X ",rx_buffer[i]);
    }

    unsigned char fBuffer[4] = {rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6]};
    float f;
    memcpy (&f, fBuffer, 4);

    printf("%f\n", f);
}

int isCfcVAlid(unsigned char *command, int size) {
    short calculated_crc = calcula_CRC(command, size - 2);

    short crc;
    memcpy (&crc, &command[size - 2], 2);

    printf("CFC Calculado: [%hi]\n", calculated_crc);
    printf("CFC Recebido: [%hi]\n", crc);

    if(crc == calculated_crc) return 1;
    return 0;

}

int getUart(int uart0_filestream, char command) {
    int rx_length = 0;
    int count = 0;

    while(rx_length < 5 && count != 3){
        sleep(1);
        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[10];
        rx_length = read(uart0_filestream, (void*)rx_buffer, 9);      //Filestream, buffer to store in, number of bytes to read (max)

        if (rx_length < 0)
        {
            printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
        }
        else if (rx_length == 0)
        {
            printf("Nenhum dado disponível.\n"); //No data waiting
        }
        else
        {
            if(isCfcVAlid(rx_buffer, rx_length))
            {
                printRx(rx_buffer, rx_length);
                return 1;
            }
        }

        count++;
    }

    return 0;
}

void sendUart(int uart0_filestream, char command) {
    int size = 0;

    if(command == '1' || command == '2' ||command == '3'){
        size = 7;
    } else if(command == '4' || command == '5' || command == '9') {
        size = 11;
    } else {
        size = 8;
    }

    unsigned char tx_buffer [size+2];

    switch (command)
    {
    case '1':
        memcpy (&tx_buffer, GET_INTERN_TEMP, size);
        break;
    case '2':
        memcpy (&tx_buffer, GET_REF_TEMP, size);
        break;
    case '3':
        memcpy (&tx_buffer, GET_USER_INPUT, size);
        break;
    case '4':
        memcpy (&tx_buffer, SEND_CONTROL_SIGNAL, size);
        break;
    case '5':
        memcpy (&tx_buffer, SEND_REF_SIGNAL, size);
        break;
    case '6':
        memcpy (&tx_buffer, SEND_SYSTEM_STATUS, size);
        break;
    case '7':
        memcpy (&tx_buffer, SEND_TEMP_MODE, size);
        break;
    case '8':
        memcpy (&tx_buffer, SEND_FUNC_STATUS, size);
        break;
    case '9':
        memcpy (&tx_buffer, SEND_ENV_TEMP, size);
        break;
    }

    short crc = calcula_CRC(tx_buffer, size);

    tx_buffer[size] = crc & 0xFF;
    tx_buffer[size+1] = crc >> 8;

    int count = write(uart0_filestream, tx_buffer, sizeof(tx_buffer));

    printf("Bytes enviados: [%d]\n", count);
}

int useUart(char command) {
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode

    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else
    {
        printf("UART inicializada!\n");
    }

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    sendUart(uart0_filestream, command);
    int result = getUart(uart0_filestream, command);

    close(uart0_filestream);

    printf("UART finalizada!\n");

    return result;
}
