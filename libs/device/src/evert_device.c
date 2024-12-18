#include "adc.h"
#include "evert_device.h"

static EVERT_DEVICE_BaseDeviceTypeDef device;

//
// #region "Alarm Matrix"
//

/// @brief Device Emergency Shutdown Alarms
static const EVERT_DEVICE_AlarmRegister1IndexTypeDef device_emergency_shutdown_alarms[5] = {
    DARI_CPU_OVERTEMPERATURE_CRITICAL,
    DARI_CPU_UNDERTEMPERATURE_CRITICAL,
    DARI_CPU_OVERVOLTAGE_CRITICAL,
    DARI_CPU_UNDERVOLTAGE_CRITICAL,
    DARI_CRITICAL_COMMS};

/// @brief Device Non-Operational Alarms
static const EVERT_DEVICE_AlarmRegister1IndexTypeDef device_non_operational_alarms[12] = {
    DARI_BOOT_SELFTEST,
    DARI_HAL_ERROR,
    DARI_HAL_CAN_ERROR,
    DARI_HAL_ADC_ERROR,
    DARI_HAL_ADC_OVERRUN,
    DARI_HAL_DMA_ERROR,
    DARI_HAL_FLASH_ERROR,
    DARI_HAL_I2C_ERROR,
    DARI_HAL_TIM_ERROR,
    DARI_HAL_UART_ERROR,
    DARI_ISR1_CYCLES,
    DARI_ISR2_CYCLES};

/// @brief Device Operational Warning Alarms
static const EVERT_DEVICE_AlarmRegister1IndexTypeDef device_operational_warning_alarms[4] = {
    DARI_CPU_OVERTEMPERATURE_WARNING,
    DARI_CPU_UNDERTEMPERATURE_WARNING,
    DARI_CPU_OVERVOLTAGE_WARNING,
    DARI_CPU_UNDERVOLTAGE_WARNING};

//
// #endregion "Alarm Matrix"
//

//
// #region "Forward Declarations for Device State Machine"
//

static void EVERT_DEVICE_State_Init();
void EVERT_DEVICE_State_Set(const EVERT_DEVICE_StateScopeTypeDef scope, const EVERT_DEVICE_StateTypeDef state);
EVERT_DEVICE_StateTypeDef EVERT_DEVICE_State_Get(const EVERT_DEVICE_StateScopeTypeDef scope);
static void EVERT_DEVICE_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state);
static void EVERT_DEVICE_OnEnterState_Booting();
static void EVERT_DEVICE_OnEnterState_BootingAdc();
static void EVERT_DEVICE_OnEnterState_BootingComms();
static void EVERT_DEVICE_OnEnterState_BootingDone();
static void EVERT_DEVICE_OnEnterState_HandshakeAnnouncing();
static void EVERT_DEVICE_OnEnterState_HandshakeAcknowledged(const EVERT_DEVICE_StateTypeDef previous_state);
static void EVERT_DEVICE_OnEnterState_Operational(const EVERT_DEVICE_StateTypeDef previous_state);
static void EVERT_DEVICE_OnEnterState_OperationalWarning(const EVERT_DEVICE_StateTypeDef previous_state);
static void EVERT_DEVICE_OnEnterState_NonOperational(const EVERT_DEVICE_StateTypeDef previous_state);
static void EVERT_DEVICE_OnEnterState_EmergencyShutdown(const EVERT_DEVICE_StateTypeDef previous_state);

//
// #endregion "Forward Declarations for Device State Machine"
//

//
// #region "Device"
//

/// @brief Initialize the Device
void EVERT_DEVICE_Init()
{
    // Alarm Registers
    device.AlarmRegister1.reg = 0;

    // State Group
    EVERT_DEVICE_State_Init();

    // Task Scheduler
    EVERT_TASK_SCHEDULER_Init();
    EVERT_TASK_SCHEDULER_SetTaskSendAnnouncementInterval(EVERT_SETTING_DEVICE_TASK_SEND_ANNOUNCEMENT_INTERVAL);
    EVERT_TASK_SCHEDULER_SetTaskSendDataInterval(EVERT_SETTING_DEVICE_TASK_SEND_DATA_INTERVAL);
    EVERT_TASK_SCHEDULER_SetTaskSendDeviceStatusInterval(EVERT_SETTING_DEVICE_TASK_SEND_STATUS_INTERVAL);
    EVERT_TASK_SCHEDULER_SetTaskSendPingInterval(EVERT_SETTING_DEVICE_TASK_SEND_PING_INTERVAL);

    // State
    EVERT_DEVICE_State_Set(SS_INTERNAL, DS_BOOTING_ADC);
}

void EVERT_DEVICE_SetVersionInfo(const uint8_t major, const uint8_t minor, const uint8_t patch)
{
    device.DeviceVersionInfo.major = major;
    device.DeviceVersionInfo.minor = minor;
    device.DeviceVersionInfo.patch = patch;
}

/// @brief Update the Device
/// @param tick_ms Deltatime tick in milliseconds
void EVERT_DEVICE_Update(const uint32_t elapsed_ms, const uint32_t delta_ms)
{
    // Update the Task Scheduler
    EVERT_TASK_SCHEDULER_Update(delta_ms);

    if (device.StateGroup.Result == DS_BOOTING_ADC)
    {
        if (elapsed_ms > EVERT_CONSTANT_DEVICE_ADC_BOOT_TIME)
        {
            EVERT_DEVICE_State_Set(SS_INTERNAL, DS_BOOTING_COMMS);
        }
    }
}

//
// #endregion "Device"
//

//
// #region "Device State Machine"
//

/// @brief Initialize the Device State Group
/// @param state_group Pointer to the Device State Group
static void EVERT_DEVICE_State_Init()
{
    device.StateGroup.Internal = DS_BOOTING;
    device.StateGroup.Propagated = DS_UNKNOWN;
    device.StateGroup.Result = DS_BOOTING;
}

/// @brief Get the Device State
/// @param state_group Pointer to the Device State Group
/// @param scope The scope of the state to get
/// @return The Device State
EVERT_DEVICE_StateTypeDef EVERT_DEVICE_State_Get(const EVERT_DEVICE_StateScopeTypeDef scope)
{
    switch (scope)
    {
    case SS_INTERNAL:
        return device.StateGroup.Internal;

    case SS_PROPAGATED:
        return device.StateGroup.Propagated;

    default:
        return DS_UNKNOWN;
    }

    return device.StateGroup.Result;
}

/// @brief Check the Device Alarm Register for any alarms
/// @return The Device State
EVERT_DEVICE_StateTypeDef EVERT_DEVICE_Alarm_Check()
{
    // Emergency Shutdown - Critical Alarms
    if (EVERT_SR_IsAnyBitSet(&device.AlarmRegister1, device_emergency_shutdown_alarms, sizeof(device_emergency_shutdown_alarms) / sizeof(device_emergency_shutdown_alarms[0])))
    {
        return DS_EMERGENCY_SHUTDOWN;
    }

    // Non-operational - Generic/HAL Alarms
    if (EVERT_SR_IsAnyBitSet(&device.AlarmRegister1, device_non_operational_alarms, sizeof(device_non_operational_alarms) / sizeof(device_non_operational_alarms[0])))
    {
        return DS_NON_OPERATIONAL;
    }

    // Warning Alarms - Operational Warning
    if (EVERT_SR_IsAnyBitSet(&device.AlarmRegister1, device_operational_warning_alarms, sizeof(device_operational_warning_alarms) / sizeof(device_operational_warning_alarms[0])))
    {
        return DS_OPERATIONAL_WARNING;
    }

    // No alarm - Operational
    return DS_OPERATIONAL;
}

/// @brief Set the Device State
/// @param state_group Pointer to the Device State Group
/// @param scope The scope of the state to set
/// @param state The Device State to set
void EVERT_DEVICE_State_Set(const EVERT_DEVICE_StateScopeTypeDef scope, const EVERT_DEVICE_StateTypeDef state)
{
    // Cache the old resultant state
    EVERT_DEVICE_StateTypeDef old_state = device.StateGroup.Result;

    // Set the new state based on scope
    switch (scope)
    {
    case SS_INTERNAL:
        device.StateGroup.Internal = state;
        break;

    case SS_PROPAGATED:
        device.StateGroup.Propagated = state;
        break;

    default:
        break;
    }

    // Update the resultant state
    if (device.StateGroup.Propagated == DS_NON_OPERATIONAL || device.StateGroup.Propagated == DS_EMERGENCY_SHUTDOWN)
    {
        device.StateGroup.Result = device.StateGroup.Propagated;
    }
    else
    {
        device.StateGroup.Result = device.StateGroup.Internal;
    }

    // TODO: Add logic for state propagation based on unknown/ping timeout

    // If the resultant state changed from the old state, call the OnDeviceStateChange method
    if (device.StateGroup.Result != old_state)
    {
        EVERT_DEVICE_OnDeviceStateChange(device.StateGroup.Result, old_state);
    }
}

static void EVERT_DEVICE_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state)
{
    switch (new_state)
    {
    case DS_UNKNOWN:
        break;

    case DS_BOOTING:
        EVERT_DEVICE_OnEnterState_Booting();
        break;

    case DS_BOOTING_ADC:
        EVERT_DEVICE_OnEnterState_BootingAdc();
        break;

    case DS_BOOTING_COMMS:
        EVERT_DEVICE_OnEnterState_BootingComms();
        break;

    case DS_BOOTING_DONE:
        EVERT_DEVICE_OnEnterState_BootingDone();
        break;

    case DS_HANDSHAKE_ANNOUNCING:
        EVERT_DEVICE_OnEnterState_HandshakeAnnouncing();
        break;

    case DS_HANDSHAKE_ACKNOWLEDGED:
        EVERT_DEVICE_OnEnterState_HandshakeAcknowledged(old_state);
        break;

    case DS_OPERATIONAL:
        EVERT_DEVICE_OnEnterState_Operational(old_state);
        break;

    case DS_OPERATIONAL_WARNING:
        EVERT_DEVICE_OnEnterState_OperationalWarning(old_state);
        break;

    case DS_NON_OPERATIONAL:
        EVERT_DEVICE_OnEnterState_NonOperational(old_state);
        break;

    case DS_EMERGENCY_SHUTDOWN:
        EVERT_DEVICE_OnEnterState_EmergencyShutdown(old_state);
        break;

    default:
        break;
    }

    // Derived class callback
    EVERT_DEVICE_Derived_OnDeviceStateChange(new_state, old_state);
}

static void EVERT_DEVICE_OnEnterState_Booting()
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_Booting();
}

static void EVERT_DEVICE_OnEnterState_BootingAdc()
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_BootingAdc();
}

static void EVERT_DEVICE_OnEnterState_BootingComms()
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_BootingComms();
}

static void EVERT_DEVICE_OnEnterState_BootingDone()
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_BootingDone();

    // Set the device state to Handshake Announcing
    EVERT_DEVICE_State_Set(SS_INTERNAL, DS_HANDSHAKE_ANNOUNCING);
}

static void EVERT_DEVICE_OnEnterState_HandshakeAnnouncing()
{
    // Start the announcement task
    EVERT_TASK_SCHEDULER_ResumeTask(EVERT_TASK_SEND_ANNOUNCEMENT);

    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_HandshakeAnnouncing();
}

static void EVERT_DEVICE_OnEnterState_HandshakeAcknowledged(const EVERT_DEVICE_StateTypeDef previous_state)
{
    // Stop the announcement task
    EVERT_TASK_SCHEDULER_PauseTask(EVERT_TASK_SEND_ANNOUNCEMENT);

    // Start the ping, data and status task
    EVERT_TASK_SCHEDULER_ResumeTask(EVERT_TASK_SEND_DATA);
    EVERT_TASK_SCHEDULER_ResumeTask(EVERT_TASK_SEND_DEVICE_STATUS);
    EVERT_TASK_SCHEDULER_ResumeTask(EVERT_TASK_SEND_PING);

    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_HandshakeAcknowledged(previous_state);
    EVERT_DEVICE_State_Set(SS_INTERNAL, DS_NON_OPERATIONAL);
}

static void EVERT_DEVICE_OnEnterState_Operational(const EVERT_DEVICE_StateTypeDef previous_state)
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_Operational(previous_state);
}

static void EVERT_DEVICE_OnEnterState_OperationalWarning(const EVERT_DEVICE_StateTypeDef previous_state)
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_OperationalWarning(previous_state);
}

static void EVERT_DEVICE_OnEnterState_NonOperational(const EVERT_DEVICE_StateTypeDef previous_state)
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_NonOperational(previous_state);
}

static void EVERT_DEVICE_OnEnterState_EmergencyShutdown(const EVERT_DEVICE_StateTypeDef previous_state)
{
    // Derived class callback
    EVERT_DEVICE_Derived_OnEnterState_EmergencyShutdown(previous_state);
}

__weak void EVERT_DEVICE_Derived_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state)
{
    UNUSED(new_state);
    UNUSED(old_state);
}
__weak void EVERT_DEVICE_Derived_OnEnterState_Booting() {}
__weak void EVERT_DEVICE_Derived_OnEnterState_BootingAdc() {}
__weak void EVERT_DEVICE_Derived_OnEnterState_BootingComms() {}
__weak void EVERT_DEVICE_Derived_OnEnterState_BootingDone() {}
__weak void EVERT_DEVICE_Derived_OnEnterState_HandshakeAnnouncing() {}
__weak void EVERT_DEVICE_Derived_OnEnterState_HandshakeAcknowledged(const EVERT_DEVICE_StateTypeDef previous_state)
{
    UNUSED(previous_state);
}
__weak void EVERT_DEVICE_Derived_OnEnterState_Operational(const EVERT_DEVICE_StateTypeDef previous_state)
{
    UNUSED(previous_state);
}
__weak void EVERT_DEVICE_Derived_OnEnterState_OperationalWarning(const EVERT_DEVICE_StateTypeDef previous_state)
{
    UNUSED(previous_state);
}
__weak void EVERT_DEVICE_Derived_OnEnterState_NonOperational(const EVERT_DEVICE_StateTypeDef previous_state)
{
    UNUSED(previous_state);
}
__weak void EVERT_DEVICE_Derived_OnEnterState_EmergencyShutdown(const EVERT_DEVICE_StateTypeDef previous_state)
{
    UNUSED(previous_state);
}

//
// #endregion "Device State Machine"
//