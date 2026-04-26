#pragma once

#include "stm32u5xx_hal.h"
#include "imu.h"
#include "logger.h"
#include "fall_config.h"
#include <stdint.h>

typedef enum {
    EVENT_NONE = 0,
    EVENT_FREE_FALL,
    EVENT_IMPACT,
    EVENT_TIMEOUT,
    EVENT_FALL_CONFIRMED
} fall_event_t;

typedef enum {
    FALL_STATE_NORMAL = 0,
    FALL_STATE_FALLING,
    FALL_STATE_POST_IMPACT
} fall_state_t;

fall_event_t detect_fall(const imu_data_t*);