#ifndef EVERT_BOOST_CONVERTER_MPPT_H_
#define EVERT_BOOST_CONVERTER_MPPT_H_

#include "boost_converter.h"

extern volatile float32_t fi_voltage_in;
extern volatile float32_t fi_voltage_out;
extern volatile float32_t fi_current_in;
extern volatile float32_t fi_power_in;

void EVERT_BOOST_CONVERTER_MpptInit(void);
void EVERT_BOOST_CONVERTER_MpptRun(void);

#endif // EVERT_BOOST_CONVERTER_MPPT_H_