#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "mcp342x.h"

static const char *TAG = "mcp342x_example";

#define MCP3424_1_ADC_ADDR (MCP342X_A0GND_A1GND)
#define I2C_MASTER_SCL_IO (GPIO_NUM_22)
#define I2C_MASTER_SDA_IO (GPIO_NUM_21)

#define I2C_MASTER_NUM (I2C_NUM_0)
#define I2C_MASTER_TX_BUF_LEN (0) // disabled
#define I2C_MASTER_RX_BUF_LEN (0) // disabled
#define I2C_MASTER_FREQ_HZ (100000)

smbus_info_t *smbus_info;
mcp342x_info_t *mcp342x_info;
int16_t result16;

static void i2c_master_init(void)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(
        I2C_MASTER_NUM,
        conf.mode,
        I2C_MASTER_RX_BUF_LEN,
        I2C_MASTER_TX_BUF_LEN,
        0);
}

static void mcp3424_init(void)
{
    esp_err_t err;
    mcp342x_config_t config;
    config.channel = MCP342X_CHANNEL_1;
    config.conversion_mode = MCP342X_MODE_ONESHOT;
    config.sample_rate = MCP342X_SRATE_12BIT;
    config.gain = MCP342X_GAIN_1X;

    smbus_info = smbus_malloc();
    err = smbus_init(smbus_info, I2C_MASTER_NUM, MCP3424_1_ADC_ADDR);
    ESP_ERROR_CHECK(err);
    smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS);

    assert(smbus_info != NULL);
    mcp342x_info = mcp342x_malloc();
    err = mcp342x_init(mcp342x_info, smbus_info, config);
    ESP_ERROR_CHECK(err);
}

// void app_main()
void c_app_main()
{
    esp_err_t err;

    ESP_LOGI(TAG, "MCP3424 C Example");
    ESP_LOGI(TAG, "Start heap size: %d", esp_get_free_heap_size());

    i2c_master_init();
    mcp3424_init();

    ESP_LOGI(TAG, "Config after init: %02x", mcp342x_info->config);

    err = mcp342x_general_call(smbus_info, MCP342X_GC_RESET);
    ESP_ERROR_CHECK(err);

    while (true)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        mcp342x_start_new_conversion(mcp342x_info, smbus_info, MCP342X_CHANNEL_1);
        mcp342x_read_result_16(mcp342x_info, smbus_info, &result16);
        ESP_LOGI(TAG, "Result: %d\n", result16);
    }
}
