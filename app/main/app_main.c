/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "tlv_parser.h"
#include "version.h"

const EventBits_t k_bit_start = (1 << 0);
const EventBits_t k_bit_connected = (1 << 1);

wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

wifi_config_t wifi_config = {
    .sta = {.ssid = "NETGEAR24", .password = "dizzynest268"},
};

// Event handling declarations
EventGroupHandle_t wifi_event_group;

void on_sta_event(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    xEventGroupSetBits(wifi_event_group, *(EventBits_t *)handler_arg);
}

uint8_t rx_buf[1024] = {'\0'};

char packet_value[1024] = {'\0'};

TLVParser_t parser;
TLVPacket_t packet = {.type = 0, .length = 0, .value = packet_value};

void app_main()
{
    printf("App Version %d.%d.%d#%s\n", APP_MAJOR, APP_MINOR, APP_BUG, APP_HASH);

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ", chip_info.cores);

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // This function has to be called or else nothing works. I don't know what
    // its really doing yet.
    tcpip_adapter_init();

    // Some status information is conveyed via esp events, e.g. those defined
    // in esp_wifi_types.h. FreeRTOS event groups are used on top of these to
    // monitor the driver status and allow the application to progress. We
    // monitor two of these events to connect to an access point. First,
    // initialize and start the wifi driver. Once this has completed then
    // attempt to connect to an access point.
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START,
                                               &on_sta_event, (void *)&k_bit_start));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,
                                               &on_sta_event, (void *)&k_bit_connected));

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    xEventGroupWaitBits(wifi_event_group, k_bit_start, 1, 1, 10000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    xEventGroupWaitBits(wifi_event_group, k_bit_connected, 1, 1,
                        10000 / portTICK_PERIOD_MS);

    int sock_server;
    int sock_client;

    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server < 1)
    {
        printf("Could not create socket.");
    }

    struct sockaddr_in addr_server;
    struct sockaddr_in addr_client;

    socklen_t server_length = sizeof(struct sockaddr_in);
    socklen_t client_length = sizeof(struct sockaddr_in);

    memset(&addr_server, 0, sizeof(struct sockaddr_in));
    memset(&addr_client, 0, sizeof(struct sockaddr_in));
    struct in_addr in_addr_server = {.s_addr = INADDR_ANY};

    addr_server.sin_len = sizeof(struct in_addr);
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(7777);
    addr_server.sin_addr = in_addr_server;

    int err;
    printf("Attempting to bind socket...\n");
    err = bind(sock_server, (struct sockaddr *)&addr_server, server_length);
    printf("Bind result: %d\n", err);

    err = listen(sock_server, 1);
    printf("Listen result: %d\n", err);

    TLVParser_Init(&parser);

    for (;;)
    {
        sock_client =
            accept(sock_server, (struct sockaddr *)&addr_client, &client_length);
        if (sock_client < 0)
        {
            printf("%d: Could not accept client.\n", sock_client);
        }
        else
        {
            int num_tx = 0;
            do
            {
                num_tx = recv(sock_client, rx_buf, 1024, 0);
                for (int i = 0; i < num_tx; i++)
                {
                    if (TLVParser_Parse(&parser, &packet, rx_buf[i]))
                    {
                        printf("%s:%d\t%s\n", inet_ntoa(addr_client.sin_addr),
                               ntohs(addr_client.sin_port), packet.value);
                    }
                }
            } while (num_tx > 0);
            close(sock_client);
        }
    }

    for (;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
