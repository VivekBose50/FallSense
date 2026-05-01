#pragma once

#include "main.h"
#include "filter.h"
#include <stdint.h>

#define IMU_ADDR (0x6B << 1)
#define CTRL1_XL      0x10
#define CTRL2_G       0x11
#define ODR_416HZ     (0x6 << 4)
#define FS_2G         (0x0 << 2)
#define FS_250DPS         (0x0 << 2)

#define ACC_SENSITIVITY 0.000061f
#define GYRO_SENSITIVITY 0.00875f  // 250 dps

#define MAIN_LOOP_PERIOD_MS 10    // ~100 Hz loop
#define IMU_ACTIVATION_PERIOD_MS 10    // ~100 Hz loop

typedef struct {
	float ax, ay, az;
	float gx, gy, gz;
	float acc_mag;
	float gyro_mag;
} imu_data_t;

/**
 * @brief IMU Initialization Function
 */
void imu_init(void);

/**
 * @brief filter and calculates magnitude of accelerometer and gyroscope data

 */
void imu_process(imu_data_t *data);
