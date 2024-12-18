#include <stdlib.h>
#include <stdint.h>
#include "task_scheduler.h"

static EVERT_TASK_SCHEDULER_HandlerTypeDef task_scheduler;

void EVERT_TASK_SCHEDULER_Init()
{
    task_scheduler.OnTaskCallback[EVERT_TASK_SEND_ANNOUNCEMENT] = EVERT_TASK_SCHEDULER_OnTaskSendAnnouncement;
    task_scheduler.OnTaskCallback[EVERT_TASK_SEND_DATA] = EVERT_TASK_SCHEDULER_OnTaskSendData;
    task_scheduler.OnTaskCallback[EVERT_TASK_SEND_DEVICE_STATUS] = EVERT_TASK_SCHEDULER_OnTaskSendDeviceStatus;
    task_scheduler.OnTaskCallback[EVERT_TASK_SEND_PING] = EVERT_TASK_SCHEDULER_OnTaskSendPing;

    for (int i = 0; i < 4; i++)
    {
        task_scheduler.TaskEnabled[i] = false;
        task_scheduler.TaskIntervalMs[i] = 0;
        task_scheduler.TaskCounterMs[i] = 0;
    }
}

void EVERT_TASK_SCHEDULER_Update(const uint32_t delta_ms)
{
    for (int i = 0; i < 4; i++)
    {
        if (task_scheduler.TaskEnabled[i])
        {
            task_scheduler.TaskCounterMs[i] += delta_ms;

            if (task_scheduler.TaskCounterMs[i] >= task_scheduler.TaskIntervalMs[i])
            {
                task_scheduler.TaskCounterMs[i] = 0;
                task_scheduler.OnTaskCallback[i]();
            }
        }
    }
}

void EVERT_TASK_SCHEDULER_PauseTask(const EVERT_TASK_SCHEDULER_TaskTypeDef task)
{
    task_scheduler.TaskEnabled[task] = false;
}

void EVERT_TASK_SCHEDULER_ResumeTask(const EVERT_TASK_SCHEDULER_TaskTypeDef task)
{
    task_scheduler.TaskEnabled[task] = true;
}

void EVERT_TASK_SCHEDULER_SetTaskSendAnnouncementInterval(const uint32_t interval_ms)
{
    task_scheduler.TaskIntervalMs[EVERT_TASK_SEND_ANNOUNCEMENT] = interval_ms;

    if (task_scheduler.TaskEnabled[EVERT_TASK_SEND_ANNOUNCEMENT] && task_scheduler.TaskCounterMs[EVERT_TASK_SEND_ANNOUNCEMENT] >= interval_ms)
    {
        task_scheduler.TaskCounterMs[EVERT_TASK_SEND_ANNOUNCEMENT] = 0;
        task_scheduler.OnTaskCallback[EVERT_TASK_SEND_ANNOUNCEMENT]();
    }
}

void EVERT_TASK_SCHEDULER_SetTaskSendDataInterval(const uint32_t interval_ms)
{
    task_scheduler.TaskIntervalMs[EVERT_TASK_SEND_DATA] = interval_ms;

    if (task_scheduler.TaskEnabled[EVERT_TASK_SEND_DATA] && task_scheduler.TaskCounterMs[EVERT_TASK_SEND_DATA] >= interval_ms)
    {
        task_scheduler.TaskCounterMs[EVERT_TASK_SEND_DATA] = 0;
        task_scheduler.OnTaskCallback[EVERT_TASK_SEND_DATA]();
    }
}

void EVERT_TASK_SCHEDULER_SetTaskSendDeviceStatusInterval(const uint32_t interval_ms)
{
    task_scheduler.TaskIntervalMs[EVERT_TASK_SEND_DEVICE_STATUS] = interval_ms;

    if (task_scheduler.TaskEnabled[EVERT_TASK_SEND_DEVICE_STATUS] && task_scheduler.TaskCounterMs[EVERT_TASK_SEND_DEVICE_STATUS] >= interval_ms)
    {
        task_scheduler.TaskCounterMs[EVERT_TASK_SEND_DEVICE_STATUS] = 0;
        task_scheduler.OnTaskCallback[EVERT_TASK_SEND_DEVICE_STATUS]();
    }
}

void EVERT_TASK_SCHEDULER_SetTaskSendPingInterval(const uint32_t interval_ms)
{
    task_scheduler.TaskIntervalMs[EVERT_TASK_SEND_PING] = interval_ms;

    if (task_scheduler.TaskEnabled[EVERT_TASK_SEND_PING] && task_scheduler.TaskCounterMs[EVERT_TASK_SEND_PING] >= interval_ms)
    {
        task_scheduler.TaskCounterMs[EVERT_TASK_SEND_PING] = 0;
        task_scheduler.OnTaskCallback[EVERT_TASK_SEND_PING]();
    }
}

__weak void EVERT_TASK_SCHEDULER_OnTaskSendAnnouncement(void) {}
__weak void EVERT_TASK_SCHEDULER_OnTaskSendData(void) {}
__weak void EVERT_TASK_SCHEDULER_OnTaskSendDeviceStatus(void) {}
__weak void EVERT_TASK_SCHEDULER_OnTaskSendPing(void) {}