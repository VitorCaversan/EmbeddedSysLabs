#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "cmsis_os2.h" // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"

#include "Uart/uart.h"
#include "SysTick/sysTick.h"

#define CMD_MAX_SIZE 6
#define MSG_MAX_SIZE 16
#define FLOOR_QTY    16

typedef enum DataIdx
{
    ELEVATOR = 0,
    CMD,
    FLOOR
} DataIdx;

typedef enum Direction
{
    IDLE = 0,
    UP,
    DOWN
} Direction;

typedef struct ElevatorInfo
{
    unsigned char upRequests[FLOOR_QTY];
    unsigned char downRequests[FLOOR_QTY];
    unsigned char cabinRequests[FLOOR_QTY];
    Direction     direction;
    unsigned long currFloor;
    unsigned long destFloor;
    bool          isDoorOpen;
} ElevatorInfo;

osThreadId_t elevadorE_id, elevadorC_id, elevadorD_id;
osThreadId_t uartRead_id, uartWrite_id;

osMessageQueueId_t queueE_id, queueC_id, queueD_id, queueUartTx_id, queueUartRx_id;

static void queueCmd(char *cmd, char elevator, char command);

static size_t strnlen(const char *str, size_t max_len);

static bool isFloorEvent(const char *msg);

static void
addFloorToDestinations(ElevatorInfo *pElevatorInfo, unsigned char floor, unsigned char dir);

static void clearFloorRequest(ElevatorInfo *pElevatorInfo, unsigned char floor);

static void decideNextMove(ElevatorInfo *pElevatorInfo);

static bool anyRequestsAboveToGoUp(ElevatorInfo *pElevatorInfo);

static bool anyRequestsAboveToGoDown(ElevatorInfo *pElevatorInfo);

static bool anyRequestsBelowToGoUp(ElevatorInfo *pElevatorInfo);

static bool anyRequestsBelowToGoDown(ElevatorInfo *pElevatorInfo);

static bool anyRequestsInGeneral(ElevatorInfo *pElevatorInfo);

void elevatorE_task(void *argument)
{
    static ElevatorInfo elevatorInfo;

    static unsigned char extBtnFloor    = 0;
    static unsigned char extBtnDir      = 0;
    static unsigned char signalFeedback = 0;

    osStatus_t status = osOK;

    char cmd[CMD_MAX_SIZE] = {0};
    char msg[MSG_MAX_SIZE] = {0};

    queueCmd(cmd, 'e', 'r');
    osDelay(5000);
    queueCmd(cmd, 'e', 'f');

    while (1)
    {
        status = osMessageQueueGet(queueE_id, msg, NULL, osWaitForever);

        if (status != osOK)
        {
            continue;
        }

        if (msg[CMD] == 'E') // External btns
        {
            sscanf(msg, "%*c%*c%hhu%c", &extBtnFloor, &extBtnDir);

            addFloorToDestinations(&elevatorInfo, extBtnFloor, extBtnDir);
        }
        else if ((msg[CMD] == 'I') && (msg[FLOOR] >= 'a') && (msg[FLOOR] <= 'p')) // Internal btns
        {
            unsigned char floorIdx = 0;
            if (msg[FLOOR] >= 'a' && msg[FLOOR] <= 'p')
            {
                floorIdx = (msg[FLOOR] - 'a');
                addFloorToDestinations(&elevatorInfo, floorIdx, 'c');
            }
        }
        else if (isFloorEvent(msg))
        {
            sscanf(msg, "%*c%hhu", &signalFeedback);
            elevatorInfo.currFloor = signalFeedback;

            if (elevatorInfo.currFloor == elevatorInfo.destFloor)
            {
                clearFloorRequest(&elevatorInfo, elevatorInfo.currFloor);

                queueCmd(cmd, 'e', 'p');
                queueCmd(cmd, 'e', 'a');
                osDelay(8000);
            }
        }
        else
        {
            if (msg[CMD] == 'A')
            {
                elevatorInfo.isDoorOpen = true;
            }
        }

        decideNextMove(&elevatorInfo);
        if (elevatorInfo.direction == UP)
        {
            if (elevatorInfo.isDoorOpen)
            {
                queueCmd(cmd, 'e', 'f');
                elevatorInfo.isDoorOpen = false;
            }
            queueCmd(cmd, 'e', 's');
        }
        else if (elevatorInfo.direction == DOWN)
        {
            if (elevatorInfo.isDoorOpen)
            {
                queueCmd(cmd, 'e', 'f');
                elevatorInfo.isDoorOpen = false;
            }
            queueCmd(cmd, 'e', 'd');
        }
        else if (elevatorInfo.direction == IDLE)
        {
            queueCmd(cmd, 'e', 'p');
        }
    }
}

void elevatorC_task(void *argument)
{
    static ElevatorInfo elevatorInfo;

    static unsigned char extBtnFloor    = 0;
    static unsigned char extBtnDir      = 0;
    static unsigned char signalFeedback = 0;

    osStatus_t status = osOK;

    char cmd[CMD_MAX_SIZE] = {0};
    char msg[MSG_MAX_SIZE] = {0};

    queueCmd(cmd, 'c', 'r');
    osDelay(5000);
    queueCmd(cmd, 'c', 'f');

    while (1)
    {
        status = osMessageQueueGet(queueC_id, msg, NULL, osWaitForever);

        if (status != osOK)
        {
            continue;
        }

        if (msg[CMD] == 'E') // External btns
        {
            sscanf(msg, "%*c%*c%hhu%c", &extBtnFloor, &extBtnDir);

            addFloorToDestinations(&elevatorInfo, extBtnFloor, extBtnDir);
        }
        else if ((msg[CMD] == 'I') && (msg[FLOOR] >= 'a') && (msg[FLOOR] <= 'p')) // Internal btns
        {
            unsigned char floorIdx = 0;
            if (msg[FLOOR] >= 'a' && msg[FLOOR] <= 'p')
            {
                floorIdx = (msg[FLOOR] - 'a');
                addFloorToDestinations(&elevatorInfo, floorIdx, 'c');
            }
        }
        else if (isFloorEvent(msg))
        {
            sscanf(msg, "%*c%hhu", &signalFeedback);
            elevatorInfo.currFloor = signalFeedback;

            if (elevatorInfo.currFloor == elevatorInfo.destFloor)
            {
                clearFloorRequest(&elevatorInfo, elevatorInfo.currFloor);

                queueCmd(cmd, 'c', 'p');
                queueCmd(cmd, 'c', 'a');
                osDelay(8000);
            }
        }
        else
        {
            if (msg[CMD] == 'A')
            {
                elevatorInfo.isDoorOpen = true;
            }
        }

        decideNextMove(&elevatorInfo);
        if (elevatorInfo.direction == UP)
        {
            if (elevatorInfo.isDoorOpen)
            {
                queueCmd(cmd, 'c', 'f');
                elevatorInfo.isDoorOpen = false;
            }
            queueCmd(cmd, 'c', 's');
        }
        else if (elevatorInfo.direction == DOWN)
        {
            if (elevatorInfo.isDoorOpen)
            {
                queueCmd(cmd, 'c', 'f');
                elevatorInfo.isDoorOpen = false;
            }
            queueCmd(cmd, 'c', 'd');
        }
        else if (elevatorInfo.direction == IDLE)
        {
            queueCmd(cmd, 'c', 'p');
        }
    }
}

void elevatorD_task(void *argument)
{
    static ElevatorInfo elevatorInfo;

    static unsigned char extBtnFloor    = 0;
    static unsigned char extBtnDir      = 0;
    static unsigned char signalFeedback = 0;

    osStatus_t status = osOK;

    char cmd[CMD_MAX_SIZE] = {0};
    char msg[MSG_MAX_SIZE] = {0};

    queueCmd(cmd, 'd', 'r');
    osDelay(5000);
    queueCmd(cmd, 'd', 'f');

    while (1)
    {
        status = osMessageQueueGet(queueD_id, msg, NULL, osWaitForever);

        if (status != osOK)
        {
            continue;
        }

        if (msg[CMD] == 'E') // External btns
        {
            sscanf(msg, "%*c%*c%hhu%c", &extBtnFloor, &extBtnDir);

            addFloorToDestinations(&elevatorInfo, extBtnFloor, extBtnDir);
        }
        else if ((msg[CMD] == 'I') && (msg[FLOOR] >= 'a') && (msg[FLOOR] <= 'p')) // Internal btns
        {
            unsigned char floorIdx = 0;
            if (msg[FLOOR] >= 'a' && msg[FLOOR] <= 'p')
            {
                floorIdx = (msg[FLOOR] - 'a');
                addFloorToDestinations(&elevatorInfo, floorIdx, 'c');
            }
        }
        else if (isFloorEvent(msg))
        {
            sscanf(msg, "%*c%hhu", &signalFeedback);
            elevatorInfo.currFloor = signalFeedback;

            if (elevatorInfo.currFloor == elevatorInfo.destFloor)
            {
                clearFloorRequest(&elevatorInfo, elevatorInfo.currFloor);

                queueCmd(cmd, 'd', 'p');
                queueCmd(cmd, 'd', 'a');
                osDelay(8000);
            }
        }
        else
        {
            if (msg[CMD] == 'A')
            {
                elevatorInfo.isDoorOpen = true;
            }
        }

        decideNextMove(&elevatorInfo);
        if (elevatorInfo.direction == UP)
        {
            if (elevatorInfo.isDoorOpen)
            {
                queueCmd(cmd, 'd', 'f');
                elevatorInfo.isDoorOpen = false;
            }
            queueCmd(cmd, 'd', 's');
        }
        else if (elevatorInfo.direction == DOWN)
        {
            if (elevatorInfo.isDoorOpen)
            {
                queueCmd(cmd, 'd', 'f');
                elevatorInfo.isDoorOpen = false;
            }
            queueCmd(cmd, 'd', 'd');
        }
        else if (elevatorInfo.direction == IDLE)
        {
            queueCmd(cmd, 'd', 'p');
        }
    }
}

void uartRead_task(void *arg)
{
    char          msg[MSG_MAX_SIZE] = {0};
    unsigned long bytesRead         = 0;
    while (1)
    {
        osMessageQueueGet(queueUartRx_id, msg, NULL, osWaitForever);

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

        osDelay(100);

        memset(msg, 0, sizeof(msg));
    }
}

void uartWrite_task(void *arg)
{
    char msg[CMD_MAX_SIZE] = {0};
    while (1)
    {
        osMessageQueueGet(queueUartTx_id, msg, NULL, osWaitForever);

        if (strnlen(msg, 4) < 4)
        {
            uart_sendString((const char *)msg);
        }
        else
        {
            uart_sendArray((const char *)msg, CMD_MAX_SIZE);
        }

        memset(msg, 0, sizeof(msg));
    }
}

int main(void)
{
    sysTick_setClkFreq();

    osKernelInitialize();

    queueUartTx_id = osMessageQueueNew(30, sizeof(char) * CMD_MAX_SIZE, NULL);
    queueUartRx_id = osMessageQueueNew(30, sizeof(char) * MSG_MAX_SIZE, NULL);

    uart_setupUart();

    uartRead_id  = osThreadNew(uartRead_task, NULL, NULL);
    uartWrite_id = osThreadNew(uartWrite_task, NULL, NULL);

    queueE_id = osMessageQueueNew(15, sizeof(char) * MSG_MAX_SIZE, NULL);
    queueC_id = osMessageQueueNew(15, sizeof(char) * MSG_MAX_SIZE, NULL);
    queueD_id = osMessageQueueNew(15, sizeof(char) * MSG_MAX_SIZE, NULL);

    elevadorE_id = osThreadNew(elevatorE_task, NULL, NULL);
    elevadorC_id = osThreadNew(elevatorC_task, NULL, NULL);
    elevadorD_id = osThreadNew(elevatorD_task, NULL, NULL);

    osKernelStart();

    while (1)
        ;
}

static void queueCmd(char *cmd, char elevator, char command)
{
    cmd[ELEVATOR] = elevator;
    cmd[CMD]      = command;
    cmd[CMD + 1]  = 0x0D;
    osMessageQueuePut(queueUartTx_id, cmd, 0, 0);
}

static size_t strnlen(const char *str, size_t max_len)
{
    size_t len = 0;
    while (*str != '\0' && len < max_len)
    {
        len++;
        str++;
    }

    return len;
}

static bool isFloorEvent(const char *msg)
{
    unsigned long msgLen = 0;
    char         *msgPtr = (char *)msg;

    while ((*msgPtr != '\r') && (*msgPtr != '\0'))
    {
        msgLen++;
        msgPtr++;
    }

    if (msgLen <= 2)
    {
        return (msg[1] >= '0' && msg[1] <= '9');
    }
    else if (msgLen <= 3)
    {
        return (msg[1] > '0' && msg[1] <= '2') && (msg[2] >= '0' && msg[2] <= '9');
    }

    return false;
}

static void
addFloorToDestinations(ElevatorInfo *pElevatorInfo, unsigned char floor, unsigned char dir)
{
    switch (dir)
    {
        case 's':
        {
            if (floor < FLOOR_QTY)
            {
                pElevatorInfo->upRequests[floor] = 1;
            }
        }
        break;

        case 'd':
        {
            if (floor < FLOOR_QTY)
            {
                pElevatorInfo->downRequests[floor] = 1;
            }
        }
        break;

        case 'c':
        default:
        {
            if (floor < FLOOR_QTY)
            {
                pElevatorInfo->cabinRequests[floor] = 1;
            }
        }
        break;
    }
}

static void clearFloorRequest(ElevatorInfo *pElevatorInfo, unsigned char floor)
{
    pElevatorInfo->upRequests[floor]    = 0;
    pElevatorInfo->downRequests[floor]  = 0;
    pElevatorInfo->cabinRequests[floor] = 0;
}

static void decideNextMove(ElevatorInfo *pElevatorInfo)
{
    if (pElevatorInfo->direction == UP)
    {
        if (anyRequestsAboveToGoUp(pElevatorInfo))
        {
            pElevatorInfo->direction = UP;
        }
        else if (anyRequestsAboveToGoDown(pElevatorInfo))
        {
            pElevatorInfo->direction = UP;
        }
        else if (anyRequestsBelowToGoDown(pElevatorInfo))
        {
            pElevatorInfo->direction = DOWN;
        }
        else
        {
            pElevatorInfo->direction = IDLE;
        }
    }
    else if (pElevatorInfo->direction == DOWN)
    {
        if (anyRequestsBelowToGoDown(pElevatorInfo))
        {
            pElevatorInfo->direction = DOWN;
        }
        else if (anyRequestsBelowToGoUp(pElevatorInfo))
        {
            pElevatorInfo->direction = DOWN;
        }
        else if (anyRequestsAboveToGoUp(pElevatorInfo))
        {
            pElevatorInfo->direction = UP;
        }
        else
        {
            pElevatorInfo->direction = IDLE;
        }
    }
    else
    {
        if (anyRequestsInGeneral(pElevatorInfo))
        {
            if (pElevatorInfo->destFloor > pElevatorInfo->currFloor)
            {
                pElevatorInfo->direction = UP;
            }
            else if (pElevatorInfo->destFloor < pElevatorInfo->currFloor)
            {
                pElevatorInfo->direction = DOWN;
            }
            else
            {
                pElevatorInfo->direction = IDLE;
            }
        }
    }
}

static bool anyRequestsAboveToGoUp(ElevatorInfo *pElevatorInfo)
{
    if (pElevatorInfo->currFloor >= (FLOOR_QTY - 1))
    {
        return false; // No floors above the top floor
    }

    for (unsigned char itr = (pElevatorInfo->currFloor + 1); itr < FLOOR_QTY; itr++)
    {
        if (pElevatorInfo->upRequests[itr] || pElevatorInfo->cabinRequests[itr])
        {
            pElevatorInfo->destFloor = itr;
            return true;
        }
    }

    return false;
}

static bool anyRequestsAboveToGoDown(ElevatorInfo *pElevatorInfo)
{
    if (pElevatorInfo->currFloor >= (FLOOR_QTY - 1))
    {
        return false; // No floors above the top floor
    }

    for (unsigned char itr = (pElevatorInfo->currFloor + 1); itr < FLOOR_QTY; itr++)
    {
        if (pElevatorInfo->downRequests[itr] || pElevatorInfo->cabinRequests[itr])
        {
            pElevatorInfo->destFloor = itr;
            return true;
        }
    }

    return false;
}

static bool anyRequestsBelowToGoUp(ElevatorInfo *pElevatorInfo)
{
    if (pElevatorInfo->currFloor == 0)
    {
        return false; // No floors below the ground floor
    }

    for (long itr = (pElevatorInfo->currFloor - 1); itr >= 0; itr--)
    {
        if (pElevatorInfo->upRequests[itr] || pElevatorInfo->cabinRequests[itr])
        {
            pElevatorInfo->destFloor = itr;
            return true;
        }
    }

    return false;
}

static bool anyRequestsBelowToGoDown(ElevatorInfo *pElevatorInfo)
{
    if (pElevatorInfo->currFloor == 0)
    {
        return false; // No floors below the ground floor
    }

    for (long itr = (pElevatorInfo->currFloor - 1); itr >= 0; itr--)
    {
        if (pElevatorInfo->downRequests[itr] || pElevatorInfo->cabinRequests[itr])
        {
            pElevatorInfo->destFloor = itr;
            return true;
        }
    }

    return false;
}

static bool anyRequestsInGeneral(ElevatorInfo *pElevatorInfo)
{
    for (long itr = 0; itr < FLOOR_QTY; itr++)
    {
        if (pElevatorInfo->upRequests[itr] || pElevatorInfo->downRequests[itr] ||
            pElevatorInfo->cabinRequests[itr])
        {
            if (pElevatorInfo->currFloor != itr)
            {
                pElevatorInfo->destFloor = itr;
            }

            return true;
        }
    }

    return false;
}