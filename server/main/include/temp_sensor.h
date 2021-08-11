#ifndef __TEMP_SENSOR_H__
#define __TEMP_SENSOR_H__

#include "esp_system.h"
#include "freertos/FreeRTOS.h"

#define I2C_SCL_PIN GPIO_NUM_22
#define I2C_SDA_PIN GPIO_NUM_23

void app_init_i2c();
uint16_t read_temp_sensor();
uint16_t get_temp_c();
void poll_temp_sensor(void*);

#endif
