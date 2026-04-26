#include "fall_detection.h"

fall_event_t detect_fall(float acc_mag, float gyro_mag) {
	static fall_state_t state = FALL_STATE_NORMAL;
	fall_event_t event = EVENT_NONE;
	static uint32_t t0 = 0;

	switch (state) {
	case FALL_STATE_NORMAL: // Normal
		if (acc_mag < FALL_FREEFALL_ACC_THRESHOLD)   // free fall
		{
			event = EVENT_FREE_FALL;
			state = FALL_STATE_FALLING;
			t0 = HAL_GetTick();
			log_info("FREE FALL--------- : A=%.2f", acc_mag);
		}
		break;

	case FALL_STATE_FALLING: // Falling
		if (acc_mag > FALL_IMPACT_ACC_THRESHOLD)   // impact
		{
			event = EVENT_IMPACT;
			state = FALL_STATE_POST_IMPACT;
			t0 = HAL_GetTick();
			log_info(" IMPACT--------- : A=%.2f", acc_mag);
		} else if (HAL_GetTick() - t0 > FALL_FREEFALL_ACC_TIMEOUT_MS) {
			state = FALL_STATE_NORMAL; // timeout
			event = EVENT_TIMEOUT;
		}
		break;

	case FALL_STATE_POST_IMPACT: // Post-impact (verify stillness)
		if (HAL_GetTick() - t0 > FALL_STILLNESS_TIME_MS) {
			if (acc_mag < FALL_STILL_ACC_THRESHOLD_G
					&& gyro_mag < FALL_STILL_GYRO_THRESHOLD_DPS) {

				log_info("STILL--------- : A=%.2f  G=%.2f", acc_mag, gyro_mag);
				event = EVENT_FALL_CONFIRMED;
			}
			state = FALL_STATE_NORMAL;      //Reset state
		}
		break;
	}
	return event;
}
