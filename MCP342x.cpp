/*
Craft Metrics

This product includes software developed by the
Craft Metrics (https://craftmetrics.ca/).
*/

#include <string.h>
#include <esp_log.h>
#include <smbus.h>
#include "MCP342x.h"

static const char *TAG = "MCP342x";

static bool _is_init(const mcp342x_info_t *mcp342x_info_ptr)
{
    bool ok = false;
    if (mcp342x_info_ptr != NULL)
    {
        if (mcp342x_info_ptr->init)
        {
            ok = true;
        }
        else
        {
            ESP_LOGE(TAG, "mcp342x_info is not initialised");
        }
    }
    else
    {
        ESP_LOGE(TAG, "mcp342x_info is NULL");
    }
    return ok;
}

/*-----------------------------------------------------------
* PUBLIC C API
*----------------------------------------------------------*/

mcp342x_info_t *MCP342xMalloc(void)
{
    mcp342x_info_t *mcp342x_info = (mcp342x_info_t *)malloc(sizeof(*mcp342x_info));
    if (mcp342x_info != NULL)
    {
        memset(mcp342x_info, 0, sizeof(*mcp342x_info));
        ESP_LOGD(TAG, "malloc mcp342x_info %p", mcp342x_info);
    }
    else
    {
        ESP_LOGE(TAG, "malloc mcp342x_info failed");
    }
    return mcp342x_info;
}

void MCP342xFree(mcp342x_info_t **mcp342x_info_ptr_ptr)
{
    if (mcp342x_info_ptr_ptr != NULL && (*mcp342x_info_ptr_ptr != NULL))
    {
        ESP_LOGD(TAG, "free mcp342x_info_t %p", *mcp342x_info_ptr_ptr);
        free(*mcp342x_info_ptr_ptr);
        *mcp342x_info_ptr_ptr = NULL;
    }
    else
    {
        ESP_LOGE(TAG, "free mcp342x_info_t failed");
    }
}

esp_err_t MCP342xGeneralCall(const smbus_info_t *smbus_info_ptr, mcp342x_general_call_t call)
{
    return smbus_write_byte(smbus_info_ptr, MCP342X_GC_START, call);
}

esp_err_t MCP342xInit(mcp342x_info_t *mcp342x_info_ptr, smbus_info_t *smbus_info_ptr, mcp342x_config_t in_config)
{
    esp_err_t err = ESP_FAIL;
    if (mcp342x_info_ptr != NULL)
    {
        mcp342x_info_ptr->smbus_info = smbus_info_ptr;
        mcp342x_info_ptr->config = (in_config.conversion_mode |
                                    in_config.channel |
                                    in_config.gain |
                                    in_config.sample_rate);
        // Test connection
        err = smbus_send_byte(smbus_info_ptr, mcp342x_info_ptr->config | MCP342X_CNTRL_NOTHING);
    }
    else
    {
        ESP_LOGE(TAG, "mcp342x_info is NULL");
    }
    return err;
}

esp_err_t MCP342xWriteToConfigRegister(mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, mcp342x_config_t in_config)
{
    esp_err_t err = ESP_FAIL;
    if (_is_init(mcp342x_info_ptr))
    {
        mcp342x_info_ptr->config = (in_config.channel |
                                    in_config.conversion_mode |
                                    in_config.gain |
                                    in_config.sample_rate);

        err = smbus_send_byte(smbus_info_ptr, mcp342x_info_ptr->config);
    }
    return err;
}

esp_err_t MCP342xStartConversion(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr)
{
    esp_err_t err = ESP_FAIL;
    if (_is_init(mcp342x_info_ptr))
    {
        err = smbus_send_byte(smbus_info_ptr, mcp342x_info_ptr->config | MCP342X_CNTRL_TRIGGER);
    }
    return err;
}

uint8_t MCP342xGetResult16(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int16_t *i16_result_ptr)
{
    uint8_t status_byte;
    union {
        uint8_t ui8[2];
        int16_t i16;
    } buffer;

    do
    {
        ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[1]));
        ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[0]));
        ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &status_byte));
    } while (status_byte & MCP342X_CNTRL_MASK);
    *i16_result_ptr = buffer.i16;

    return status_byte;
}

uint8_t MCP342xGetResult18(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int32_t *i32_result_ptr)
{
    uint8_t status_byte;
    union {
        uint8_t ui8[3];
        int32_t i32;
    } buffer;

    do
    {
        ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[2]));
        ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[1]));
        ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[0]));
        ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &status_byte));
    } while (status_byte & MCP342X_CNTRL_MASK);

    *i32_result_ptr = buffer.i32;

    return status_byte;
}

uint8_t MCP342xGetResultSingle16(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int16_t *i16_result_ptr)
{
    uint8_t status_byte;
    union {
        uint8_t ui8[2];
        int16_t i16;
    } buffer;

    ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[1]));
    ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[0]));
    ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &status_byte));

    if (status_byte == MCP342X_CNTRL_UPDATED)
        *i16_result_ptr = buffer.i16;

    return status_byte;
}

uint8_t MCP342xGetResultSingle18(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int32_t *i32_result_ptr)
{
    uint8_t status_byte;
    union {
        uint8_t ui8[3];
        int32_t i32;
    } buffer;

    ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[2]));
    ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[1]));
    ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &buffer.ui8[0]));
    ESP_ERROR_CHECK(smbus_receive_byte(smbus_info_ptr, &status_byte));

    if (status_byte == MCP342X_CNTRL_UPDATED)
        *i32_result_ptr = buffer.i32;

    return status_byte;
}

/*
#ifdef __cplusplus

MCP342x::MCP342x(mcp342x_address_t in_address)
{
    this->address = in_address;
}

void MCP342x::Configure(mcp342x_config_t in_config)
{
    this->config = (in_config.conversion_mode |
                    in_config.channel |
                    in_config.gain |
                    in_config.sample_rate);
}

uint8_t MCP342x::GetConfig(void)
{
    return this->config;
}

esp_err_t MCP342x::StartConversion()
{
    // Startbus transmission
}

#endif // __cplusplus
*/