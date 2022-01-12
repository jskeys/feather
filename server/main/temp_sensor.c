#include "temp_sensor.h"

#include "driver/i2c.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"

uint16_t temp_c = 0;

void app_init_i2c()
{
    // Move this outside eventually.
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 200000,
    };

    i2c_cmd_handle_t i2c_cmd_handle;

    // ?I2C_NUM_?
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));

    // Create the command link handle
    i2c_cmd_handle = i2c_cmd_link_create();
    // Enable high resolution mode
    ESP_ERROR_CHECK(i2c_master_start(i2c_cmd_handle));
    ESP_ERROR_CHECK(i2c_master_write_byte(i2c_cmd_handle, 0x90, 0));
    ESP_ERROR_CHECK(i2c_master_write_byte(i2c_cmd_handle, 0x01, 0));
    ESP_ERROR_CHECK(i2c_master_write_byte(i2c_cmd_handle, 0x60, 0));
    ESP_ERROR_CHECK(i2c_master_stop(i2c_cmd_handle));
    // Begin the sequence
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, i2c_cmd_handle, 10000));
    i2c_cmd_link_delete(i2c_cmd_handle);

    // Create the command link handle
    i2c_cmd_handle = i2c_cmd_link_create();
    // Set the address register to 0 to read back sensor value
    ESP_ERROR_CHECK(i2c_master_start(i2c_cmd_handle));
    ESP_ERROR_CHECK(i2c_master_write_byte(i2c_cmd_handle, 0x90, 0));
    ESP_ERROR_CHECK(i2c_master_write_byte(i2c_cmd_handle, 0x00, 0));
    ESP_ERROR_CHECK(i2c_master_stop(i2c_cmd_handle));
    // Begin the sequence
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, i2c_cmd_handle, 10000));
    i2c_cmd_link_delete(i2c_cmd_handle);
}

uint16_t read_temp_sensor()
{
    i2c_cmd_handle_t i2c_cmd_handle;

    uint8_t temp_msb;
    uint8_t temp_lsb;
    uint16_t temp = 0;

    i2c_cmd_handle = i2c_cmd_link_create();
    // Read back the temperature sensor
    ESP_ERROR_CHECK(i2c_master_start(i2c_cmd_handle));
    ESP_ERROR_CHECK(i2c_master_write_byte(i2c_cmd_handle, 0x91, 0));
    ESP_ERROR_CHECK(i2c_master_read_byte(i2c_cmd_handle, &temp_msb, 0));
    ESP_ERROR_CHECK(i2c_master_read_byte(i2c_cmd_handle, &temp_lsb, 1));
    ESP_ERROR_CHECK(i2c_master_stop(i2c_cmd_handle));
    // Begin the sequence
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, i2c_cmd_handle, 5000));
    // Delete the command link
    i2c_cmd_link_delete(i2c_cmd_handle);

    temp = (temp | temp_msb) << 8;
    temp = (temp | temp_lsb);

    return temp;
}

void poll_temp_sensor(void* args)
{
    app_init_i2c();
    for (;;)
    {
        temp_c = read_temp_sensor();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

uint16_t get_temp_c() { return temp_c; }
