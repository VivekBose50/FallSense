#include "fall_detection.h"

fall_event_t detect_fall(const imu_data_t* imu) {
	static fall_state_t state = FALL_STATE_NORMAL;
	fall_event_t event = EVENT_NONE;
	static uint32_t t0 = 0;

	switch (state) {
	case FALL_STATE_NORMAL: 
        // FREE FALL: low acceleration + losing vertical support
		if (imu->acc_mag < FALL_FREEFALL_ACC_THRESHOLD &&
                imu->az < FALL_FREEFALL_Z_THRESHOLD)
		{
			event = EVENT_FREE_FALL;
			state = FALL_STATE_FALLING;
			t0 = HAL_GetTick();
			log_info("FREE FALL--------- : A=%.2f az=%.2f", imu->acc_mag, imu->az);
		}
		break;

	case FALL_STATE_FALLING:
        // IMPACT: sudden high acceleration + rotation spike
		if (imu->acc_mag > FALL_IMPACT_ACC_THRESHOLD &&
                imu->gyro_mag > FALL_IMPACT_GYRO_THRESHOLD)
		{
			event = EVENT_IMPACT;
			state = FALL_STATE_POST_IMPACT;
			t0 = HAL_GetTick();
			log_info(" IMPACT--------- : A=%.2f gyro=%.2f", imu->acc_mag, imu->gyro_mag);
		} else if (HAL_GetTick() - t0 > FALL_FREEFALL_ACC_TIMEOUT_MS) {
			state = FALL_STATE_NORMAL;       //Reset
			event = EVENT_TIMEOUT;
		}
		break;

	case FALL_STATE_POST_IMPACT:
        // STILLNESS CHECK after impact
		if (HAL_GetTick() - t0 > FALL_STILLNESS_TIME_MS) {
			if (imu->acc_mag < FALL_STILL_ACC_THRESHOLD_G
					&& imu->gyro_mag < FALL_STILL_GYRO_THRESHOLD_DPS) {

				log_info("STILL--------- : A=%.2f  G=%.2f", imu->acc_mag, imu->gyro_mag);
				event = EVENT_FALL_CONFIRMED;
			}
			state = FALL_STATE_NORMAL;      //Reset state
		}
		break;
	}
	return event;
}
