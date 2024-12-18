#include "boost_converter_alarms.h"

EVERT_SR_RegisterTypeDefinitionTypeDef alarm_register;

void EVERT_BOOST_CONVERTER_InitAlarmRegister()
{
    EVERT_SR_Init(&alarm_register);
}

__weak void EVERT_BOOST_CONVERTER_AlarmChangedState(EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index, bool is_set)
{
    UNUSED(alarm_index);
    UNUSED(is_set);
}

static bool EVERT_BOOST_CONVERTER_CheckLowAlarm(const float32_t value, const float32_t hysteresis, const float32_t low_warning, const float32_t low_critical,
                                                const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_low_warning, EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_low_critical)
{
    if (value < low_critical)
    {
        if (EVERT_SR_SetBit(&alarm_register, alarm_index_low_critical))
        {
            EVERT_BOOST_CONVERTER_AlarmChangedState(alarm_index_low_critical, true);
        }

        return true;
    }
    else if (value < low_warning)
    {
        if (EVERT_SR_SetBit(&alarm_register, alarm_index_low_warning))
        {
            EVERT_BOOST_CONVERTER_AlarmChangedState(alarm_index_low_warning, true);
        }

        return true;
    }
    else if (value > (low_warning + hysteresis))
    {
        if (EVERT_SR_ClearBit(&alarm_register, alarm_index_low_warning))
        {
            EVERT_BOOST_CONVERTER_AlarmChangedState(alarm_index_low_warning, false);
        }

        return false;
    }

    return false;
}

static bool EVERT_BOOST_CONVERTER_CheckHighAlarm(const float32_t value, const float32_t hysteresis, const float32_t high_warning, const float32_t high_critical,
                                                 const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_high_warning, EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_high_critical)
{
    if (value > high_critical)
    {
        if (EVERT_SR_SetBit(&alarm_register, alarm_index_high_critical))
        {
            EVERT_BOOST_CONVERTER_AlarmChangedState(alarm_index_high_critical, true);
        }

        return true;
    }
    else if (value > high_warning)
    {
        if (EVERT_SR_SetBit(&alarm_register, alarm_index_high_warning))
        {
            EVERT_BOOST_CONVERTER_AlarmChangedState(alarm_index_high_warning, true);
        }

        return true;
    }
    else if (value < (high_warning - hysteresis))
    {
        if (EVERT_SR_ClearBit(&alarm_register, alarm_index_high_warning))
        {
            EVERT_BOOST_CONVERTER_AlarmChangedState(alarm_index_high_warning, false);
        }

        return false;
    }

    return false;
}

static bool EVERT_BOOST_CONVERTER_Check4BandAlarm(const float32_t value, const float32_t hysteresis, const float32_t low_warning, const float32_t low_critical,
                                                  const float32_t high_warning, const float32_t high_critical, const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_low_warning,
                                                  const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_low_critical, const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_high_warning,
                                                  const EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index_high_critical)
{
    if (EVERT_BOOST_CONVERTER_CheckLowAlarm(value, hysteresis, low_warning, low_critical, alarm_index_low_warning, alarm_index_low_critical))
    {
        return true;
    }
    else if (EVERT_BOOST_CONVERTER_CheckHighAlarm(value, hysteresis, high_warning, high_critical, alarm_index_high_warning, alarm_index_high_critical))
    {
        return true;
    }

    return false;
}

void EVERT_BOOST_CONVERTER_AlarmCheck()
{
    // Voltage In
    EVERT_BOOST_CONVERTER_Check4BandAlarm(fi_voltage_in, EVERT_CONSTRAINT_BC_VOLTAGE_IN_HYSTERESIS,
                                          EVERT_CONSTRAINT_BC_VOLTAGE_IN_LOW_WARNING, EVERT_CONSTRAINT_BC_VOLTAGE_IN_LOW_CRITICAL,
                                          EVERT_CONSTRAINT_BC_VOLTAGE_IN_HIGH_WARNING, EVERT_CONSTRAINT_BC_VOLTAGE_IN_HIGH_CRITICAL,
                                          BCAI_UNDERVOLTAGE_IN_WARNING, BCAI_UNDERVOLTAGE_IN_CRITICAL, BCAI_OVERVOLTAGE_IN_WARNING, BCAI_OVERVOLTAGE_IN_CRITICAL);

    // Voltage Out
    EVERT_BOOST_CONVERTER_CheckHighAlarm(fi_voltage_out, EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HYSTERESIS,
                                         EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HIGH_WARNING, EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HIGH_CRITICAL,
                                         BCAI_OVERVOLTAGE_OUT_WARNING, BCAI_OVERVOLTAGE_OUT_CRITICAL);

    // Current In
    EVERT_BOOST_CONVERTER_CheckHighAlarm(fi_current_in, EVERT_CONSTRAINT_BC_CURRENT_HYSTERESIS,
                                         EVERT_CONSTRAINT_BC_CURRENT_HIGH_WARNING, EVERT_CONSTRAINT_BC_CURRENT_HIGH_CRITICAL,
                                         BCAI_OVERCURRENT_WARNING, BCAI_OVERCURRENT_CRITICAL);
}