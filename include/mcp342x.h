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

#ifndef ESP32_MCP342X_H
#define ESP32_MCP342X_H

#include <stdint.h>
#include <esp_system.h>
#include <esp_log.h>
#include "smbus.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*-----------------------------------------------------------
* MACROS & ENUMS
*----------------------------------------------------------*/

/** I2C Address of device
 * MCP3421, MCP3425 & MCP3426 are factory programed for any of 0x68 thru 0x6F
 * MCP3422, MCP3423, MCP3424, MCP3427 & MCP3428 addresses are controlled by address lines A0 and A1
 * each address line can be low (GND), high (VCC) or floating (FLT)
 */
typedef enum MCP342xAddress
{
    MCP342X_A0GND_A1GND = 0x68,
    MCP342X_A0GND_A1FLT = 0x69,
    MCP342X_A0GND_A1VCC = 0x6A,
    MCP342X_A0FLT_A1GND = 0x6B,
    MCP342X_A0VCC_A1GND = 0x6C,
    MCP342X_A0VCC_A1FLT = 0x6D,
    MCP342X_A0VCC_A1VCC = 0x6E,
    MCP342X_A0FLT_A1VCC = 0x6F,
} mcp342x_address_t;

/** Ready Bit definitions (bit 7)
 */
typedef enum MCP342xControl
{
    // Write in OneShot Mode
    MCP342X_CNTRL_NO_EFFECT = 0x00,
    MCP342X_CNTRL_TRIGGER_CONVERSION = 0x80,
    // Reading from register
    MCP342X_CNTRL_RESULT_NOT_UPDATED = 0x80,
    MCP342X_CNTRL_RESULT_UPDATED = 0x00,
    // Mask to get Bit 7
    MCP342X_CNTRL_MASK = 0x80,
} mcp342x_control_t;

/** Channel definitions (bit 6-5)
 * MCP3421 & MCP3425 have only the one channel and ignore this param
 * MCP3422, MCP3423, MCP3426 & MCP3427 have two channels and treat 3 & 4 as repeats of 1 & 2 respectively
 * MCP3424 & MCP3428 have all four channels
 */
typedef enum MCP342xChannel
{
    MCP342X_CHANNEL_1 = 0x00, // 0b 0000 0000
    MCP342X_CHANNEL_2 = 0x20, // 0b 0010 0000
    MCP342X_CHANNEL_3 = 0x40, // 0b 0100 0000
    MCP342X_CHANNEL_4 = 0x60, // 0b 0110 0000
    MCP342X_CHANNEL_MASK = 0x60,
} mcp342x_channel_t;

/** Conversion mode definitions (bit 4)
 */
typedef enum MCP342xConversionMode
{
    MCP342X_MODE_ONESHOT = 0x00,    // 0b 0000 0000
    MCP342X_MODE_CONTINUOUS = 0x10, // 0b 0001 0000
    MCP342X_MODE_MASK = 0x10,
} mcp342x_conversion_mode_t;

/** Sample size definitions (bit 3-2)
 * these also affect the sampling rate
 * 12-bit has a max sample rate of 240sps
 * 14-bit has a max sample rate of  60sps
 * 16-bit has a max sample rate of  15sps
 * 18-bit has a max sample rate of   3.75sps (MCP3421, MCP3422, MCP3423, MCP3424 only)
 */
typedef enum MCP342xSampleRate
{
    MCP342X_SRATE_12BIT = 0x00, // 0b 0000 0000
    MCP342X_SRATE_14BIT = 0x04, // 0b 0000 0100
    MCP342X_SRATE_16BIT = 0x08, // 0b 0000 1000
    MCP342X_SRATE_18BIT = 0x0C, // 0b 0000 1100
    MCP342X_SRATE_MASK = 0x0C,
} mcp342x_sample_rate_t;

/** Programmable Gain PGA definitions (bit 1-0)
 */
typedef enum MCP342xGain
{
    MCP342X_GAIN_1X = 0x00, // 0b 0000 0000
    MCP342X_GAIN_2X = 0x01, // 0b 0000 0001
    MCP342X_GAIN_4X = 0x02, // 0b 0000 0010
    MCP342X_GAIN_8X = 0x03, // 0b 0000 0011
    MCP342X_GAIN_MASK = 0x03,
} mcp342x_gain_t;

typedef enum MCP342xGeneralCall
{
    MCP342X_GC_START = 0x00,
    MCP342X_GC_LATCH = 0x04,
    MCP342X_GC_RESET = 0x06,
    MCP342X_GC_CONVERSION = 0x08
} mcp342x_general_call_t;

typedef enum
{
    MCP342X_STATUS_OK,
    MCP342X_STATUS_UNDERFLOW,
    MCP342X_STATUS_OVERFLOW,
    MCP342X_STATUS_I2C,
    MCP342X_STATUS_IN_PROGRESS,
    MCP342X_STATUS_TIMEOUT
} mcp342x_conversion_status_t;

/** Configuration Register values of the MCP342x device
 * Initialized with default settings partially according to
 * datasheet Section 4.1
 */
typedef struct MCP342xConfig
{
    mcp342x_channel_t channel;
    mcp342x_conversion_mode_t conversion_mode;
    mcp342x_sample_rate_t sample_rate;
    mcp342x_gain_t gain;
} mcp342x_config_t;

/** Struct for controlling a MCP342x device
 * smbus_info contains the i2c address of the device
 */
typedef struct MCP342xInfo_t
{
    bool init : 1;
    smbus_info_t *smbus_info;
    uint8_t config;
} mcp342x_info_t;

/*-----------------------------------------------------------
* DEFINITIONS
*----------------------------------------------------------*/

/**
 * @brief Construct a new MCP342x info instance.
 *        New instance should be initialised before calling other functions.
 *
 * @return Pointer to new device info instance, or NULL if it cannot be created.
 */
mcp342x_info_t *mcp342x_malloc(void);

/**
 * @brief Delete an existing MCP342x info instance.
 *
 * @param[in,out] Pointer to MCP342x info instance that will be freed and set to NULL.
 */
void mcp342x_free(mcp342x_info_t **mcp342x_info);

/**
 * @brief Initialise a MCP342x instance with the specified SMBus information.
 *
 * @param[in] mcp342x_info Pointer to MCP342x info instance.
 * @param[in] smbus_info Pointer to SMBus info instance.
 * @param[in] in_config Configuration bitmask. 
 * @return ESP_OK if successful, otherwise an error constant.
 */
esp_err_t mcp342x_init(mcp342x_info_t *mcp342x_info, smbus_info_t *smbus_info, mcp342x_config_t in_config);

/**
 * @brief Set the configuration values for the MCP342x instance
 *
 * @param[in] mcp342x_info Pointer to MCP342x info instance.
 * @param[in] in_config Configuration bitmask.
 */
void mcp342x_set_config(mcp342x_info_t *mcp342x_info_ptr, mcp342x_config_t in_config); 

/**
 * @brief Specific call to the device samples the logic status 
 *        of the Adr0 and Adr1 pins in the general call events
 * 
 * @param[in] smbus_info_ptr Pointer to smbus info instance.
 * @param[in] call General call to write.
 * 
 * @return ESP_OK if successful, otherwise an error constant.
 */
esp_err_t mcp342x_general_call(const mcp342x_info_t *mcp342x_info_ptr, mcp342x_general_call_t call);

/**
 * @brief Trigger a conversion on the MCP342x instance
 *
 * @param[in] mcp342x_info_ptr Pointer to MCP342x info instance.
 * @param[in] smbus_info_ptr Pointer to SMBus info instance.
 * 
 * @return ESP_OK if successful, otherwise an error constant.
 */
esp_err_t mcp342x_start_new_conversion(const mcp342x_info_t *mcp342x_info_ptr);

/**
 * @brief Read the result of the conversion
 *
 * @param[in] mcp342x_info_ptr Pointer to MCP342x info instance.
 * @param[in] smbus_info_ptr Pointer to SMBus info instance.
 * @param[in] result Reference to result info variable.
 * 
 * @return Conversion Status
 */
mcp342x_conversion_status_t mcp342x_read_result(const mcp342x_info_t *mcp342x_info_ptr, double *result);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace cm
{

class MCP342x
{
  public:
    MCP342x(mcp342x_address_t in_address);
    ~MCP342x();
    esp_err_t Init(i2c_port_t in_i2c_master, mcp342x_config_t in_config);
    esp_err_t GeneralCall(mcp342x_general_call_t call);
    void SetConfig(mcp342x_config_t in_config);
    esp_err_t StartNewConversion(void);
    esp_err_t StartNewConversion(mcp342x_channel_t in_channel);
    double Read(void);
    mcp342x_address_t GetAddress(void);
    mcp342x_info_t *GetInfoPtr(void);

  private:
    double GetResult();
    mcp342x_address_t address;
    mcp342x_info_t *mcp342x_info;
    double result;
};

} // namespace cm

#endif // __cplusplus

#endif // ESP32_MCP342X_H
