#ifndef EVERT_HAL_CONF_
#define EVERT_HAL_CONF_

#define __overrides __noopt
#define __noopt __attribute__((used))

// COMMUNICATION
#define EVERT_HAL_CONF_DEBUGGING (1)

// EMC230X
#define EVERT_HAL_CONF_EMC230X_ENABLE (true)
#define EVERT_HAL_CONF_EMC230X_ADDRESS (0x002C)
#define EVERT_HAL_CONF_EMC230x_TYPE (2303)

// FZ2812
#define EVERT_HAL_CONF_FZ2812_ENABLE (true)
#define EVERT_HAL_CONF_FZ2812_COUNT (2)

#endif // EVERT_HAL_CONF_