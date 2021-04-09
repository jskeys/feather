#ifndef __APP_H__
#define __APP_H__

#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef struct
{
    EventGroupHandle_t group;
    EventBits_t bit;
} AppEventArgs_t;

void app_init_gpio();
void app_init_wifi();
void _on_sta_event(void *handler_arg, esp_event_base_t base, int32_t id,
                   void *event_data);

#endif
