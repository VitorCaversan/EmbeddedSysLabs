#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "cmsis_os2.h"       // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"

#include "Uart/uart.h"
#include "SysTick/sysTick.h"

typedef enum
{
    ELEVATOR = 0,
    CMD,
    FLOOR
} DataIdx;

osThreadId_t elevadorE_id, elevadorC_id, elevadorD_id;
osThreadId_t uartRead_id, uartWrite_id;

osTimerId_t timerE_id, timerC_id, timerD_id;

osMessageQueueId_t queueE_id, queueC_id, queueD_id, queueUart_id;

static size_t strnlen(const char* str, size_t max_len);

void callbackE(void* arg)
{
    osThreadFlagsSet(elevadorE_id, 0x0001);
}

void callbackC(void* arg)
{
    osThreadFlagsSet(elevadorC_id, 0x0002);
}

void callbackD(void* arg)
{
    osThreadFlagsSet(elevadorD_id, 0x0003);
}

void elevadorE(void* argument)
{
    static uint8_t curr_floor = 0;
    static long    next_floor = -1;
    static uint8_t dest_vect[16];

    static uint8_t i;
    static uint8_t ext_btn_floor = 0;
    static uint8_t signal_feedback = 0;
    static uint8_t vect_zero = 0;
    volatile bool floor_changed = false;

    char cmd[3] = {0};
    char msg[10];

    // RESET
    cmd[ELEVATOR] = 'e'; // Qual elevador
    cmd[CMD] = 'r'; // resetar
    osMessageQueuePut(queueUart_id, cmd, 0, 0);
    osDelay(1000);

    // FECHAR
    cmd[ELEVATOR] = 'e'; // Qual elevador
    cmd[CMD] = 'f'; // fechar
    osMessageQueuePut(queueUart_id, cmd, 0, 0);

    while (1)
    {
        osMessageQueueGet(queueE_id, msg, NULL, osWaitForever);
        
        if (msg[CMD] == 'E') // BOTOES EXTERNOS
        {                                               // botao externo
            sscanf(msg, "%*c%*c%hhu%*c", &ext_btn_floor); // Auxiliar que recebe o valor do andar do
                                                        // botão externo pressionado
            dest_vect[ext_btn_floor] = 1;               // Atualiza o vetor de destino do elevador

            // Caso seja a primeira vez
            if ((next_floor == -1) || ((vect_zero == 0) && (floor_changed)))
            {
                floor_changed = false;
                next_floor = ext_btn_floor;
            }

            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }
        else if (msg[CMD] == 'I') // Inside button
        {
            unsigned char floorIdx = 0;
            if (msg[FLOOR] >= 'a' && msg[FLOOR] <= 'p')
            {
                floorIdx = (msg[FLOOR] - 'a');
                dest_vect[floorIdx] = 1;
            }

            for (i = 0; i < 16; i++)
            {
                if (dest_vect[i] == 1)
                {
                    next_floor = i;
                }
            }

            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }

        } // if do botão interno

        else if ((msg[CMD] >= '0' && msg[ELEVATOR] == 'c' && msg[CMD] <= '9') ||
                 (msg[FLOOR] >= '0' && msg[CMD] <= '2'))
        {
            floor_changed = true;

            sscanf(msg, "%*c%hhu", &signal_feedback);
            curr_floor = signal_feedback;
            if (curr_floor == next_floor)
            {
                dest_vect[curr_floor] = 0;
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 'p'; // parar
                osMessageQueuePut(queueUart_id, cmd, 0, 0);

                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 'a'; // abrir porta
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
                osTimerStart(timerE_id, 3000);
                osThreadFlagsWait(0x0002, osFlagsWaitAny, osWaitForever);

                for (i = 0; i < 16; i++)
                {
                    if (dest_vect[i] == 1)
                    {
                        next_floor = i;
                        vect_zero = 1;
                        break;
                    }
                    else
                        vect_zero = 0;
                }
            }
        } // Fim else do feedback
        else
        {
            if (msg[CMD] == 'A')
            {
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 'f'; // fechar porta
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }

        if (vect_zero == 1)
        {
            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'e'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }
    }
}

void elevadorC(void* argument)
{
    static uint8_t curr_floor = 0;
    static long    next_floor = -1;
    static uint8_t dest_vect[16];

    static uint8_t i;
    static uint8_t ext_btn_floor = 0;
    static uint8_t signal_feedback = 0;
    static uint8_t vect_zero = 0;
    volatile bool floor_changed = false;

    char cmd[3] = {0};
    char msg[10];

    // ENVIA RESET
    cmd[ELEVATOR] = 'c'; // Qual elevador
    cmd[CMD] = 'r'; // resetar
    osMessageQueuePut(queueUart_id, cmd, 0, 0);
    osDelay(1000);

    // FECHAR AS PORTAS
    cmd[ELEVATOR] = 'c';
    cmd[CMD] = 'f'; // fechar
    osMessageQueuePut(queueUart_id, cmd, 0, 0);

    while (1)
    {
        osMessageQueueGet(queueC_id, msg, NULL, osWaitForever);
        // TRATAMENTO DOS BOTÕES EXTERNOS
        if (msg[CMD] == 'E')
        {                                               // botao externo
            sscanf(msg, "%*c%*c%hhu%*c", &ext_btn_floor); // Auxiliar que recebe o valor do andar do
                                                        // botão externo pressionado
            dest_vect[ext_btn_floor] = 1;               // Atualiza o vetor de destino do elevador

            // Caso seja a primeira vez
            if ((next_floor == -1) || ((vect_zero == 0) && (floor_changed)))
            { // se for a primeira vez (next_floor = -1), se o vetor de
              // destino tiver somente zeros, ou seja, só foi pedido um andar
              // (vect_zero), se ja mudou de andar (floor_changed)
                floor_changed = false;
                next_floor = ext_btn_floor;
            }

            // manda subir caso curr_floor for menor que o próximo andar
            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            // manda descer caso curr_floor for maior que o próximo andar
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        } // if do botão externo
        else if (msg[CMD] == 'I') // Inside button
        {
            unsigned char floorIdx = 0;
            if (msg[FLOOR] >= 'a' && msg[FLOOR] <= 'p')
            {
                floorIdx = (msg[FLOOR] - 'a');
                dest_vect[floorIdx] = 1;
            }

            for (i = 0; i < 16; i++)
            {
                if (dest_vect[i] == 1)
                {
                    next_floor = i;
                }
            }

            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }

        else if ((msg[CMD] >= '0' && msg[ELEVATOR] == 'c' && msg[CMD] <= '9') ||
                 (msg[FLOOR] >= '0' && msg[CMD] <= '2'))
        {
            floor_changed = true; // Flag que vai indicar que teve mudança de andar

            sscanf(msg, "%*c%hhu", &signal_feedback);
            curr_floor = signal_feedback;
            if (curr_floor == next_floor)
            {
                dest_vect[curr_floor] = 0; // Zera a posição que ja foi atendida no vetor de destino

                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 'p'; // parar
                osMessageQueuePut(queueUart_id, cmd, 0, 0);

                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 'a'; // abrir porta
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
                osTimerStart(timerC_id, 3000);
                osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);

                for (i = 0; i < 16; i++)
                {
                    if (dest_vect[i] == 1)
                    {
                        next_floor = i;
                        vect_zero = 1;
                        break;
                    }
                    else
                        vect_zero = 0;
                }
            }
        } // Fim else do feedback
        else
        { // ultimo else são os feeback de porta aberta e porta fechada
            if (msg[CMD] == 'A')
            {
                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 'f'; // fechar porta
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }

        // Aqui lógica para comandar a subida e descida do elevador, mesmo se os botões não forem
        // mais apertados, porém o vetor de destino ainda tem elementos
        if (vect_zero == 1)
        {
            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'c'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }
    }
}

void elevadorD(void* argument)
{
    static uint8_t curr_floor = 0;
    static long    next_floor = -1;
    static uint8_t dest_vect[16];

    static uint8_t i;
    static uint8_t ext_btn_floor = 0;
    static uint8_t signal_feedback = 0;
    static uint8_t vect_zero = 0;
    volatile bool floor_changed = false;

    char cmd[3] = {0};
    char msg[10];
    char floor_letters[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};

    // ENVIA RESET
    cmd[ELEVATOR] = 'd'; // Qual elevador
    cmd[CMD] = 'r'; // resetar
    osMessageQueuePut(queueUart_id, cmd, 0, 0);
    osDelay(1000);

    // FECHAR AS PORTAS
    cmd[ELEVATOR] = 'd';
    cmd[CMD] = 'f'; // fechar
    osMessageQueuePut(queueUart_id, cmd, 0, 0);

    while (1)
    {
        osMessageQueueGet(queueD_id, msg, NULL, osWaitForever);
        // TRATAMENTO DOS BOTÕES EXTERNOS
        if (msg[CMD] == 'E')
        {                                               // botao externo
            sscanf(msg, "%*c%*c%hhu%*c", &ext_btn_floor); // Auxiliar que recebe o valor do andar do
                                                        // botão externo pressionado
            dest_vect[ext_btn_floor] = 1;               // Atualiza o vetor de destino do elevador

            // Caso seja a primeira vez
            if ((next_floor == -1) || ((vect_zero == 0) && (floor_changed)))
            {
                floor_changed = false;
                next_floor = ext_btn_floor;
            }
            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        } // if do botão externo
        else if ((msg[CMD] == 'I') && (msg[FLOOR] >= 'a') && (msg[FLOOR] <= 'p'))
        { // botao interno
            for (i = 0; i < 16; i++)
            {
                if (floor_letters[i] == msg[FLOOR])
                {
                    dest_vect[i] = 1;
                }
            }

            for (i = 0; i < 16; i++)
            {
                if (dest_vect[i] == 1)
                {
                    next_floor = i;
                }
            }

            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }

        } // if do botão interno

        else if ((msg[CMD] >= '0' && msg[CMD] <= '9') || (msg[FLOOR] >= '0' && msg[CMD] <= '2'))
        {
            floor_changed = true;

            sscanf(msg, "%*c%hhu", &signal_feedback);
            curr_floor = signal_feedback;

            if (curr_floor == next_floor)
            {
                dest_vect[curr_floor] = 0;
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 'p'; // parar
                osMessageQueuePut(queueUart_id, cmd, 0, 0);

                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 'a'; // abrir porta
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
                osTimerStart(timerD_id, 3000);
                osThreadFlagsWait(0x0003, osFlagsWaitAny, osWaitForever);

                for (i = 0; i < 16; i++)
                {
                    if (dest_vect[i] == 1)
                    {
                        next_floor = i;
                        vect_zero = 1;
                        break;
                    }
                    else
                        vect_zero = 0;
                }
            }
        } // Fim else do feedback
        else
        {
            if (msg[CMD] == 'A')
            {
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 'f'; // fechar porta
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }

        if (vect_zero == 1)
        {
            if (curr_floor < next_floor)
            {
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 's'; // subir
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
            else if (curr_floor > next_floor)
            {
                cmd[ELEVATOR] = 'd'; // Qual elevador
                cmd[CMD] = 'd'; // descer
                osMessageQueuePut(queueUart_id, cmd, 0, 0);
            }
        }
    }
}

void uartRead(void* arg)
{
    char msg[10] = {0};
    unsigned long bytesRead = 0;
    while (1)
    {
        bytesRead = uart_GetBytes(UART0_BASE, (uint8_t*)msg, sizeof(msg) - 1);
        if (msg[ELEVATOR] == 'e')
        {
            osMessageQueuePut(queueE_id, msg, 0, 0);
        }
        else if (msg[ELEVATOR] == 'c')
        {
            osMessageQueuePut(queueC_id, msg, 0, 0);
        }
        else if (msg[ELEVATOR] == 'd')
        {
            osMessageQueuePut(queueD_id, msg, 0, 0);
        }
        memset(msg, 0, sizeof(msg));
        osThreadYield(); // talvez usar osDelay(1) no lugar
    }
}

void uartWrite(void* arg)
{
    char msg[4] = {0};
    while (1)
    {
        osMessageQueueGet(queueUart_id, msg, NULL, osWaitForever);

        if (strnlen(msg, 4) < 4)
        {
            uart_sendString((const char*)msg);
        }
        else
        {
            uart_sendArray((const char*)msg, 3);
        }

        memset(msg, 0, sizeof(msg));
    }
}

int main(void)
{
    sysTick_setClkFreq();
    
    osKernelInitialize();
    
    uart_setupUart();

    uartRead_id = osThreadNew(uartRead, NULL, NULL);
    uartWrite_id = osThreadNew(uartWrite, NULL, NULL);

    queueE_id = osMessageQueueNew(15, sizeof(char) * 6, NULL);
    queueC_id = osMessageQueueNew(15, sizeof(char) * 6, NULL);
    queueD_id = osMessageQueueNew(15, sizeof(char) * 6, NULL);

    queueUart_id = osMessageQueueNew(30, sizeof(char) * 3, NULL);

    elevadorE_id = osThreadNew(elevadorE, NULL, NULL);
    elevadorC_id = osThreadNew(elevadorC, NULL, NULL);
    elevadorD_id = osThreadNew(elevadorD, NULL, NULL);

    timerE_id = osTimerNew(callbackE, osTimerOnce, NULL, NULL);
    timerC_id = osTimerNew(callbackC, osTimerOnce, NULL, NULL);
    timerD_id = osTimerNew(callbackD, osTimerOnce, NULL, NULL);

    osKernelStart();

    while (1)
        ;
}

static size_t strnlen(const char* str, size_t max_len)
{
    size_t len = 0;
    while (*str != '\0' && len < max_len)
    {
        len++;
        str++;
    }

    return len;
}
