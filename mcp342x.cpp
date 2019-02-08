/*
    Craft Metrics

    This product includes software developed by
    Craft Metrics (https://craftmetrics.ca/).

    MIT License
    Copyright (c) 2018 Craft Metrics

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <string.h>
#include <esp_log.h>
#include <smbus.h>
#include "mcp342x.h"

static const char *TAG = "mcp342x";

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

mcp342x_info_t *mcp342x_malloc(void)
{
    mcp342x_info_t *mcp342x_info = (mcp342x_info_t *)malloc(sizeof(*mcp342x_info));
    if (mcp342x_info != NULL)
    {
        memset(mcp342x_info, 0, sizeof(*mcp342x_info));
        ESP_LOGD(TAG, "malloc mcp342x_info_t %p", mcp342x_info);
    }
    else
    {
        ESP_LOGE(TAG, "malloc mcp342x_info failed");
    }
    return mcp342x_info;
}

void mcp342x_free(mcp342x_info_t **mcp342x_info_ptr_ptr)
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

esp_err_t mcp342x_general_call(const smbus_info_t *smbus_info_ptr, mcp342x_general_call_t call)
{
    return smbus_write_byte(smbus_info_ptr, MCP342X_GC_START, call);
}

esp_err_t mcp342x_init(mcp342x_info_t *mcp342x_info_ptr, smbus_info_t *smbus_info_ptr, mcp342x_config_t in_config)
{
    esp_err_t err = ESP_FAIL;
    if (mcp342x_info_ptr != NULL)
    {
        ESP_LOGD(TAG, "init mcp342x_info_t %p", mcp342x_info_ptr);
        mcp342x_info_ptr->smbus_info = smbus_info_ptr;
        mcp342x_info_ptr->config = (in_config.conversion_mode |
                                    in_config.channel |
                                    in_config.gain |
                                    in_config.sample_rate);
        // Test connection
        ESP_LOGD(TAG, "send mcp342x_info config %p", &mcp342x_info_ptr->config);
        err = smbus_send_byte(smbus_info_ptr, mcp342x_info_ptr->config | MCP342X_CNTRL_NOTHING);
    }
    else
    {
        ESP_LOGE(TAG, "mcp342x_info is NULL");
    }
    return err;
}

esp_err_t mcp342x_write_config_reg(mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, mcp342x_config_t in_config)
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

esp_err_t mcp342x_start_conversion(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr)
{
    esp_err_t err = ESP_FAIL;
    if (_is_init(mcp342x_info_ptr))
    {
        err = smbus_send_byte(smbus_info_ptr, mcp342x_info_ptr->config | MCP342X_CNTRL_TRIGGER);
    }
    return err;
}

uint8_t mcp342x_get_result_16(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int16_t *i16_result_ptr)
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

uint8_t mcp342x_get_result_18(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int32_t *i32_result_ptr)
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

uint8_t mcp342x_get_result_single_16(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int16_t *i16_result_ptr)
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

uint8_t mcp342x_get_result_single_18(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int32_t *i32_result_ptr)
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

#ifdef __cplusplus

MCP342x::MCP342x(mcp342x_address_t in_address)
{
    this->address = in_address;
}

MCP342x::~MCP342x()
{
    mcp342x_free(&this->mcp342x_info);
}

esp_err_t MCP342x::Init(i2c_port_t in_i2c_master, mcp342x_config_t in_config)
{
    smbus_info_t *smbus_info = smbus_malloc();
    smbus_init(smbus_info, in_i2c_master, this->address);
    smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS);

    assert(smbus_info != NULL);
    this->mcp342x_info = mcp342x_malloc();
    return mcp342x_init(this->mcp342x_info,
                        smbus_info,
                        in_config);
}

esp_err_t MCP342x::SetConfig(mcp342x_config_t in_config)
{
    return mcp342x_write_config_reg(this->mcp342x_info,
                                    this->mcp342x_info->smbus_info,
                                    in_config);
}

esp_err_t MCP342x::StartConversion(void)
{
    return mcp342x_start_conversion(this->mcp342x_info,
                                    this->mcp342x_info->smbus_info);
}

uint8_t MCP342x::GetResult()
{
    uint8_t ret;
    if ((this->mcp342x_info->config & MCP342X_SRATE_MASK) == MCP342X_SRATE_18BIT)
    {
        ret = mcp342x_get_result_18(this->mcp342x_info,
                                    this->mcp342x_info->smbus_info,
                                    &this->result);
    }
    else
    {
        ret = mcp342x_get_result_16(this->mcp342x_info,
                                    this->mcp342x_info->smbus_info,
                                    (int16_t *)&this->result);
    }
    return ret;
}

uint8_t MCP342x::GetResultSingle()
{
    uint8_t ret;
    if ((this->mcp342x_info->config & MCP342X_SRATE_MASK) == MCP342X_SRATE_18BIT)
    {
        ret = mcp342x_get_result_single_18(this->mcp342x_info,
                                           this->mcp342x_info->smbus_info,
                                           &this->result);
    }
    else
    {
        ret = mcp342x_get_result_single_16(this->mcp342x_info,
                                           this->mcp342x_info->smbus_info,
                                           (int16_t *)&this->result);
    }
    return ret;
}

mcp342x_address_t MCP342x::GetAddress(void)
{
    return this->address;
}

mcp342x_info_t *MCP342x::GetInfoPtr(void)
{
    return this->mcp342x_info;
}

#endif // __cplusplus
