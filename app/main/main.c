#include <stdio.h>

#include "app.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "tlv_parser.h"
#include "version.h"

char req_err[] = "ERR_REQUEST_NOT_HANDLED";

// Event handling declarations

int sock_server;
int sock_client;
struct sockaddr_in addr_server;
struct sockaddr_in addr_client;
uint8_t rx_buf[1024] = {'\0'};
uint8_t tx_buf[1024] = {'\0'};
ssize_t num_rx;
ssize_t num_tx;

TLVParser_t parser;
TLVPacket_t rx_packet = {.type = 0, .length = 0, .value = {'\0'}};
TLVPacket_t tx_packet = {.type = 0, .length = 0, .value = {'\0'}};

void print_packet(TLVPacket_t *packet)
{
    printf("TLV Packet: %d;%d;%s\n", packet->type, packet->length, packet->value);
}

ssize_t handle_packet(const TLVPacket_t *request, TLVPacket_t *response)
{
    switch (request->type)
    {
        case 0x01:
            memcpy(response, request, sizeof(TLVPacket_t));
            break;
        case 0x02:
            ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_0, *(uint32_t *)request->value));
            response->type = request->type;
            response->length = 0;
            memset(response->value, 0, TLV_MAX_LENGTH);
            break;
        case 0x03:
            ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_2, *(uint32_t *)request->value));
            response->type = request->type;
            response->length = 0;
            memset(response->value, 0, TLV_MAX_LENGTH);
            break;
        default:
            response->type = 0;
            response->length = (uint32_t)sizeof(req_err);
            memcpy(response->value, req_err, sizeof(req_err));
    }
    return response->length + 6;
}

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

    app_init_gpio();
    app_init_wifi();

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
            do
            {
                num_rx = recv(sock_client, rx_buf, 1024, 0);
                for (int i = 0; i < num_rx; i++)
                {
                    if (TLVParser_Parse(&parser, &rx_packet, rx_buf[i]))
                    {
                        print_packet(&rx_packet);
                        num_tx = handle_packet(&rx_packet, &tx_packet);
                        print_packet(&tx_packet);
                        send(sock_client, &tx_packet, num_tx, 0);
                    }
                }
            } while (num_rx > 0);
            close(sock_client);
        }
    }
}
