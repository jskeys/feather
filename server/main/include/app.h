#ifndef __APP_H__
#define __APP_H__

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define LED_LO 0
#define LED_HI 1

#define I2C_SCL_PIN GPIO_NUM_22
#define I2C_SDA_PIN GPIO_NUM_23

#define GPIO_LED_0 GPIO_NUM_13

typedef struct
{
    EventGroupHandle_t group;
    EventBits_t bit;
} AppEventArgs_t;

void app_init_i2c();
void app_init_nvs();
void app_init_gpio();
void app_init_wifi();
void _on_sta_event(void *handler_arg, esp_event_base_t base, int32_t id,
                   void *event_data);

uint16_t read_temp_sensor();

#endif
