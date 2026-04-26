#include "imu.h"

extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef *imu_i2c = &hi2c1;

static low_pass_filter_t ax_f, ay_f, az_f;
static low_pass_filter_t gx_f, gy_f, gz_f;

static int16_t combine_bytes(uint8_t l, uint8_t h) {
	return (int16_t) (h << 8 | l);
}

void imu_init(void) {

	lpf_init(&ax_f, FILTER_ALPHA);
	lpf_init(&ay_f, FILTER_ALPHA);
	lpf_init(&az_f, FILTER_ALPHA);

	lpf_init(&gx_f, FILTER_ALPHA);
	lpf_init(&gy_f, FILTER_ALPHA);
	lpf_init(&gz_f, FILTER_ALPHA);

	//Accelerometer:
	//Write to register CTRL1_XL:
	/*ODR_416HZ | FS_2G = 0x60 decoded:
	 *
	 * 0110 0000
	 * ^^^^ ---- → ODR(Output Data Rate) = 0110 = 416 Hz
	 * ---- ^^-- → FS(Full Scale) = 00 = ±2g
	 * ------ ^^ → BW(BandWidth) = default
	 */
	uint8_t acc_config = ODR_416HZ | FS_2G;

	HAL_I2C_Mem_Write(imu_i2c, IMU_ADDR, CTRL1_XL,
	I2C_MEMADD_SIZE_8BIT, &acc_config, 1, HAL_MAX_DELAY);

	uint8_t ctrl3 = 0x44;
	// 0x40 → BDU = 1
	// 0x04 → IF_INC = 1

	HAL_I2C_Mem_Write(imu_i2c, IMU_ADDR, 0x12,
	I2C_MEMADD_SIZE_8BIT, &ctrl3, 1, HAL_MAX_DELAY);

	//Gyroscope:
	//Write to register CTRL2_G:
	/*ODR_416Z | FS_250DPS = 0x60 decoded:
	 *
	 * 0110 0000
	 * ^^^^ ---- → ODR(Output Data Rate) = 0110 = 416 Hz
	 * ---- ^^-- → FS(Full Scale) = 00 = 250 dps
	 */

	uint8_t gyro_config = ODR_416HZ | FS_250DPS;  // 416 Hz, 250 dps
	HAL_I2C_Mem_Write(imu_i2c, IMU_ADDR, CTRL2_G,
	I2C_MEMADD_SIZE_8BIT, &gyro_config, 1, HAL_MAX_DELAY);

	HAL_Delay(100);
}

static void imu_read_accel(float *ax_g, float *ay_g, float *az_g) {
	uint8_t raw[6];

	HAL_I2C_Mem_Read(imu_i2c, IMU_ADDR, 0x28,
	I2C_MEMADD_SIZE_8BIT, raw, 6, HAL_MAX_DELAY);

	int16_t ax_raw = combine_bytes(raw[0], raw[1]);
	int16_t ay_raw = combine_bytes(raw[2], raw[3]);
	int16_t az_raw = combine_bytes(raw[4], raw[5]);

	*ax_g = ax_raw * ACC_SENSITIVITY;
	*ay_g = ay_raw * ACC_SENSITIVITY;
	*az_g = az_raw * ACC_SENSITIVITY;
}

static void imu_read_gyro(float *gx, float *gy, float *gz) {
	uint8_t raw[6];

	HAL_I2C_Mem_Read(imu_i2c, IMU_ADDR, 0x22,
	I2C_MEMADD_SIZE_8BIT, raw, 6, HAL_MAX_DELAY);

	int16_t gx_raw = combine_bytes(raw[0], raw[1]);
	int16_t gy_raw = combine_bytes(raw[2], raw[3]);
	int16_t gz_raw = combine_bytes(raw[4], raw[5]);

	*gx = gx_raw * GYRO_SENSITIVITY;
	*gy = gy_raw * GYRO_SENSITIVITY;
	*gz = gz_raw * GYRO_SENSITIVITY;
}

void imu_process(imu_data_t *data) {
	float ax, ay, az;
	float gx, gy, gz;

	imu_read_accel(&ax, &ay, &az);
	imu_read_gyro(&gx, &gy, &gz);

	// filtering
	data->ax = lpf_apply(&ax_f, ax);
	data->ay = lpf_apply(&ay_f, ay);
	data->az = lpf_apply(&az_f, az);
	data->gx = lpf_apply(&gx_f, gx);
	data->gy = lpf_apply(&gy_f, gy);
	data->gz = lpf_apply(&gz_f, gz);

	// Derived values
	data->acc_mag = sqrtf(
			data->ax * data->ax + data->ay * data->ay + data->az * data->az);
	data->gyro_mag = sqrtf(
			data->gx * data->gx + data->gy * data->gy + data->gz * data->gz);
}
