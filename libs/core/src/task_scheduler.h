#ifndef EVERT_TASK_SCHEDULER_H_H
#define EVERT_TASK_SCHEDULER_H_H

#include <stdint.h>
#include <stdbool.h>
#include <stm32g4xx_hal.h>

typedef enum
{
    EVERT_TASK_SEND_ANNOUNCEMENT = 0,
    EVERT_TASK_SEND_DATA = 1,
    EVERT_TASK_SEND_DEVICE_STATUS = 2,
    EVERT_TASK_SEND_PING = 3
} EVERT_TASK_SCHEDULER_TaskTypeDef;

typedef struct
{
    void (*OnTaskCallback[4])(void);
    bool TaskEnabled[4];
    uint32_t TaskIntervalMs[4];
    uint32_t TaskCounterMs[4];

} EVERT_TASK_SCHEDULER_HandlerTypeDef;

void EVERT_TASK_SCHEDULER_Init();
void EVERT_TASK_SCHEDULER_Update(const uint32_t delta_ms);
void EVERT_TASK_SCHEDULER_PauseTask(const EVERT_TASK_SCHEDULER_TaskTypeDef task);
void EVERT_TASK_SCHEDULER_ResumeTask(const EVERT_TASK_SCHEDULER_TaskTypeDef task);

void EVERT_TASK_SCHEDULER_SetTaskSendAnnouncementInterval(const uint32_t interval_ms);
void EVERT_TASK_SCHEDULER_SetTaskSendDataInterval(const uint32_t interval_ms);
void EVERT_TASK_SCHEDULER_SetTaskSendDeviceStatusInterval(const uint32_t interval_ms);
void EVERT_TASK_SCHEDULER_SetTaskSendPingInterval(const uint32_t interval_ms);

void EVERT_TASK_SCHEDULER_OnTaskSendAnnouncement(void);
void EVERT_TASK_SCHEDULER_OnTaskSendData(void);
void EVERT_TASK_SCHEDULER_OnTaskSendDeviceStatus(void);
void EVERT_TASK_SCHEDULER_OnTaskSendPing(void);

#endif // EVERT_TASK_SCHEDULER_H_H