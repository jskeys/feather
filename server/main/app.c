#include <app.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "wifi_config.h"

void app_init_nvs() { nvs_flash_init(); }

void app_init_gpio()
{
    printf("Configuring GPIO.\n");
    const gpio_config_t app_gpio_config = {
        .pin_bit_mask = 1 << GPIO_LED_0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    // Initialize GPIO. The two LEDs, which are attached to GPIO 0 and 2, will
    // convey status during bootup.
    ESP_ERROR_CHECK(gpio_config(&app_gpio_config));
    ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_0, LED_LO));
}

void app_init_wifi()
{
    wifi_config_t wifi_config = {
        .sta = {.ssid = WIFI_NAME, .password = WIFI_PASSWORD},
    };

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    const EventGroupHandle_t wifi_event_group = xEventGroupCreate();
    const EventBits_t k_bit_start = (1 << 0);
    const EventBits_t k_bit_connected = (1 << 1);

    AppEventArgs_t start_args = {.group = wifi_event_group, .bit = k_bit_start};
    AppEventArgs_t connected_args = {.group = wifi_event_group, .bit = k_bit_connected};

    // Some status information is conveyed via esp events, e.g. those defined
    // in esp_wifi_types.h. FreeRTOS event groups are used on top of these to
    // monitor the driver status and allow the application to progress. We
    // monitor two of these events to connect to an access point. First,
    // initialize and start the wifi driver. Once this has completed then
    // attempt to connect to an access point.
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // This function has to be called or else nothing works. I don't know what
    // it does.
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START,
                                               &_on_sta_event, (void *)&start_args));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,
                                               &_on_sta_event, (void *)&connected_args));

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    xEventGroupWaitBits(wifi_event_group, k_bit_start, 1, 1, 10000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    xEventGroupWaitBits(wifi_event_group, k_bit_connected, 1, 1,
                        10000 / portTICK_PERIOD_MS);
}

void _on_sta_event(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    AppEventArgs_t *arg = (AppEventArgs_t *)handler_arg;
    xEventGroupSetBits(arg->group, arg->bit);
}
