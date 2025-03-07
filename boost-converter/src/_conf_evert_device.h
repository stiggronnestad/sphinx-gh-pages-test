#ifndef EVERT_DEVICE_CONF_
#define EVERT_DEVICE_CONF_

// Constants (hardware specific)
#define EVERT_CONSTANT_DEVICE_FREQUENCY_HRCK (170000000)
#define EVERT_CONSTANT_DEVICE_MCU_VOLTAGE (3300)
#define EVERT_CONSTANT_DEVICE_ADC_BOOT_TIME (1000)

// Settings (software adjustable)
#define EVERT_SETTING_DEVICE_TASK_SEND_ANNOUNCEMENT_INTERVAL (1000)
#define EVERT_SETTING_DEVICE_TASK_SEND_DATA_INTERVAL (100)
#define EVERT_SETTING_DEVICE_TASK_SEND_PING_INTERVAL (1000)
#define EVERT_SETTING_DEVICE_TASK_SEND_STATUS_INTERVAL (100)

// Constraints (hardware specific / software adjustable)
#define EVERT_CONSTRAINT_DEVICE_CPU_TEMP_HYSTERESIS (2.5f)
#define EVERT_CONSTRAINT_DEVICE_CPU_TEMP_HIGH_WARNING (70)
#define EVERT_CONSTRAINT_DEVICE_CPU_TEMP_HIGH_CRITICAL (85)
#define EVERT_CONSTRAINT_DEVICE_CPU_TEMP_LOW_WARNING (-20)
#define EVERT_CONSTRAINT_DEVICE_CPU_TEMP_LOW_CRITICAL (-40)
#define EVERT_CONSTRAINT_DEVICE_VREF_NOMINAL (1210.0f)
#define EVERT_CONSTRAINT_DEVICE_VREF_HYSTERESIS (5.0f)
#define EVERT_CONSTRAINT_DEVICE_VREF_INT_HIGH_WARNING (EVERT_CONSTRAINT_DEVICE_VREF_NOMINAL + 35.0f)
#define EVERT_CONSTRAINT_DEVICE_VREF_INT_HIGH_CRITICAL (EVERT_CONSTRAINT_DEVICE_VREF_NOMINAL + 50.0f)
#define EVERT_CONSTRAINT_DEVICE_VREF_INT_LOW_WARNING (EVERT_CONSTRAINT_DEVICE_VREF_NOMINAL - 35.0f)
#define EVERT_CONSTRAINT_DEVICE_VREF_INT_LOW_CRITICAL (EVERT_CONSTRAINT_DEVICE_VREF_NOMINAL - 50.0f)

#endif // EVERT_DEVICE_CONF_
