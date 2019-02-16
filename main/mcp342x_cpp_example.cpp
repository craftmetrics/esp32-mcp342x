#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "mcp342x.h"

static const char *TAG = "mcp342x_example";

#define ADC_ADDR (MCP342X_A0GND_A1GND)
#define I2C_MASTER_SCL_IO (GPIO_NUM_22)
#define I2C_MASTER_SDA_IO (GPIO_NUM_21)

#define I2C_MASTER_NUM (I2C_NUM_0)
#define I2C_MASTER_TX_BUF_LEN (0) // disabled
#define I2C_MASTER_RX_BUF_LEN (0) // disabled
#define I2C_MASTER_FREQ_HZ (100000)

namespace ex
{

static cm::MCP342x mcp3424 = cm::MCP342x(ADC_ADDR);

void mcp3424_init(void)
{
    esp_err_t err;

    // Set device configuration
    mcp342x_config_t config;
    config.channel = MCP342X_CHANNEL_1;
    config.conversion_mode = MCP342X_MODE_ONESHOT;
    config.sample_rate = MCP342X_SRATE_14BIT;
    config.gain = MCP342X_GAIN_1X;

    err = mcp3424.Init(I2C_MASTER_NUM, config);
    ESP_ERROR_CHECK(err);
}

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

} // namespace ex

extern "C" void app_main()
// extern "C" void cpp_app_main()
{
    ESP_LOGI(TAG, "MCP3424 C++ Example");

    ESP_LOGI(TAG, "Before init heap size: %d", esp_get_free_heap_size());
    ex::i2c_master_init();
    ex::mcp3424_init();
    ESP_LOGI(TAG, "After init heap size: %d", esp_get_free_heap_size());

    while (true)
    {
        ESP_LOGI(TAG, "Result: %.2f", ex::mcp3424.Read(MCP342X_CHANNEL_1));
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
