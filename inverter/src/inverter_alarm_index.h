#ifndef EVERT_INVERTER_ALARM_INDEX_H_
#define EVERT_INVERTER_ALARM_INDEX_H_

typedef enum
{
    IAI_BUS_OVERVOLTAGE_CRITICAL = 0,
    IAI_BUS_OVERVOLTAGE_WARNING = 1,
    IAI_BUS_UNDERVOLTAGE_CRITICAL = 2,
    IAI_BUS_UNDERVOLTAGE_WARNING = 3,
    IAI_BUS_CAPACITOR_IMBALANCE_CRITICAL = 4,
    IAI_BUS_CAPACITOR_IMBALANCE_WARNING = 5,
    IAI_GRID_OVERVOLTAGE_CRITICAL = 6,
    IAI_GRID_OVERVOLTAGE_WARNING = 7,
    IAI_GRID_UNDERVOLTAGE_CRITICAL = 8,
    IAI_GRID_UNDERVOLTAGE_WARNING = 9,
    IAI_OVERCURRENT_CRITICAL = 10,
    IAI_OVERCURRENT_WARNING = 11,
    IAI_OVER_TEMPERATURE_AMBIENT_CRITICAL = 12,
    IAI_OVER_TEMPERATURE_AMBIENT_WARNING = 13,
    IAI_UNDER_TEMPERATURE_AMBIENT_CRITICAL = 14,
    IAI_UNDER_TEMPERATURE_AMBIENT_WARNING = 15,
    IAI_TEMPERATURE_U_CRITICAL = 16,
    IAI_TEMPERATURE_U_WARNING = 17,
    IAI_TEMPERATURE_V_CRITICAL = 18,
    IAI_TEMPERATURE_V_WARNING = 19,
    IAI_TEMPERATURE_W_CRITICAL = 20,
    IAI_TEMPERATURE_W_WARNING = 21,
} EVERT_INVERTER_AlarmIndexTypeDef;

#endif // EVERT_INVERTER_ALARM_INDEX_H_