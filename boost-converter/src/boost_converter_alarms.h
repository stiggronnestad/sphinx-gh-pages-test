#ifndef EVERT_BOOST_CONVERTER_ALARMS_H_
#define EVERT_BOOST_CONVERTER_ALARMS_H_

#include <stdbool.h>
#include "status_register.h"
#include "_conf_evert_boost_converter.h"
#include "boost_converter_readings.h"

typedef enum
{
    BCAI_OVERVOLTAGE_IN_CRITICAL = 0,
    BCAI_OVERVOLTAGE_IN_WARNING = 1,
    BCAI_UNDERVOLTAGE_IN_WARNING = 2,
    BCAI_UNDERVOLTAGE_IN_CRITICAL = 3,
    BCAI_OVERVOLTAGE_OUT_WARNING = 4,
    BCAI_OVERVOLTAGE_OUT_CRITICAL = 5,
    BCAI_OVERCURRENT_WARNING = 6,
    BCAI_OVERCURRENT_CRITICAL = 7,
    BCAI_TEMPERATURE_COIL_WARNING = 8,
    BCAI_TEMPERATURE_COIL_CRITICAL = 9,
    BCAI_TEMPERATURE_SCHOTTKY_WARNING = 10,
    BCAI_TEMPERATURE_SCHOTTKY_CRITICAL = 11,
    BCAI_TEMPERATURE_MOSFET_WARNING = 12,
    BCAI_TEMPERATURE_MOSFET_CRITICAL = 13
} EVERT_BOOST_CONVERTER_AlarmIndexTypeDef;

extern EVERT_SR_RegisterTypeDefinitionTypeDef alarm_register;

extern volatile float32_t fi_voltage_in;
extern volatile float32_t fi_voltage_out;
extern volatile float32_t fi_current_in;

static const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef BOOST_CONVERTER_ALARMS_ThrottleDown[7] = {
    BCAI_OVERVOLTAGE_IN_WARNING,
    BCAI_UNDERVOLTAGE_IN_WARNING, // TODO: Unsure about the utility here, throttling down might be counterproductive
    BCAI_OVERVOLTAGE_OUT_WARNING,
    BCAI_OVERCURRENT_WARNING, // TODO: Unsure about the utility here, throttling down might be counterproductive
    BCAI_TEMPERATURE_COIL_WARNING,
    BCAI_TEMPERATURE_SCHOTTKY_WARNING,
    BCAI_TEMPERATURE_MOSFET_WARNING};

static const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef BOOST_CONVERTER_ALARMS_Standby[7] = {
    BCAI_OVERVOLTAGE_IN_CRITICAL,
    BCAI_UNDERVOLTAGE_IN_CRITICAL,
    BCAI_OVERVOLTAGE_OUT_CRITICAL,
    BCAI_OVERCURRENT_CRITICAL,
    BCAI_TEMPERATURE_COIL_CRITICAL,
    BCAI_TEMPERATURE_SCHOTTKY_CRITICAL,
    BCAI_TEMPERATURE_MOSFET_CRITICAL};

void EVERT_BOOST_CONVERTER_InitAlarmRegister();
void EVERT_BOOST_CONVERTER_AlarmCheck();
void EVERT_BOOST_CONVERTER_AlarmChangedState(EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index, bool is_set);

#endif // EVERT_BOOST_CONVERTER_ALARMS_H_
