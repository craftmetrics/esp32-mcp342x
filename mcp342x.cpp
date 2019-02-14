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

#include "mcp342x.hpp"

namespace cm
{

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

};
