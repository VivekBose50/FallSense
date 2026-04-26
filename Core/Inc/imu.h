#pragma once

#include "main.h"
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

/**
 * @brief IMU Initialization Function
 * @param None
 * @retval None
 */
void imu_init(void);

/**
 * @brief accelerometer read Function
 * @param gx x axis output value
 * @param gy y axis output value
 * @param gz z axis output value
 * @retval None
 */
void imu_read_accel(float *ax_g, float *ay_g, float *az_g);

/**
 * @brief gyroscope read Function
 * @param gx x axis output value
 * @param gy y axis output value
 * @param gz z axis output value
 * @retval None
 */
void imu_read_gyro(float *gx, float *gy, float *gz);

