#include <stdio.h>

#include "app.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "temp_sensor.h"
#include "tlv.h"
#include "version.h"

char req_err[] = "ERR_REQUEST_NOT_HANDLED\0";

// Event handling declarations

int sock_server;
int sock_client;
struct sockaddr_in addr_server;
struct sockaddr_in addr_client;
uint8_t rx_buf[512] = {'\0'};
uint8_t tx_buf[512] = {'\0'};
ssize_t num_rx;
ssize_t num_tx;

TLVParser_t parser;

char rx_value[TLV_MAX_LENGTH] = {'0'};
char tx_value[TLV_MAX_LENGTH] = {'0'};

TLVPacket_t rx_packet = {.type = 0, .length = 0, .value = rx_value};
TLVPacket_t tx_packet = {.type = 0, .length = 0, .value = tx_value};

void print_packet(TLVPacket_t *packet)
{
    printf("0x%04x ", packet->type);
    printf("0x%04x ", packet->length);
    for (int i = 0; i < packet->length; i++)
    {
        printf("%02hhX ", *((uint8_t *)(packet->value) + i));
    }
    printf("\r\n");
}

void handle_packet(const TLVPacket_t *request, TLVPacket_t *response)
{
    uint16_t temp_c;

    response->type = request->type;
    response->length = 0;

    switch (request->type)
    {
        case 0x01:
            response->type = request->type;
            response->length = request->length;
            memcpy(response->value, request->value, request->length);
            break;
        case 0x02:
            ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_0, *(uint32_t *)request->value));
            response->type = request->type;
            break;
        case 0x03:
            ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_0, *(uint32_t *)request->value));
            response->type = request->type;
            break;
        case 0x04:
            temp_c = read_temp_sensor();
            response->length = sizeof(temp_c);
            memcpy(response->value, (void *)&temp_c, sizeof(temp_c));
            break;
        default:
            response->type = request->type;
            response->length = sizeof(req_err);
            printf("%s\n", req_err);
            memcpy(response->value, req_err, sizeof(req_err));
            break;
    }
}

void app_main()
{
    app_init_nvs();

    printf("App Version %d.%d.%d#%s \n", APP_MAJOR, APP_MINOR, APP_BUG, APP_HASH);

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ", chip_info.cores);

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    app_init_i2c();
    app_init_gpio();
    app_init_wifi();

    //    xTaskCreate(poll_temp_sensor, "TEMP", 1024, NULL, tskIDLE_PRIORITY, NULL);

    // Start a task that reads the external temperature sensor every 5 seconds.

    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server < 1)
    {
        printf("Could not create socket.");
    }

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
            tlv_initialize_parser(&parser, 0x5a55);
            do
            {
                num_rx = recv(sock_client, rx_buf, 1024, 0);
                for (int i = 0; i < num_rx; i++)
                {
                    if (tlv_process_char(&parser, &rx_packet, rx_buf[i]))
                    {
                        printf("%s: ", inet_ntoa(addr_client.sin_addr));
                        print_packet(&rx_packet);
                        handle_packet(&rx_packet, &tx_packet);
                        num_tx = tlv_write_packet_to_buffer(&tx_packet, tx_buf);
                        send(sock_client, tx_buf, num_tx, 0);
                    }
                }
            } while (num_rx > 0);
            close(sock_client);
        }
    }
}
