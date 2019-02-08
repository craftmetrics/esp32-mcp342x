/*
Craft Metrics

This product includes software developed by the
Craft Metrics (https://craftmetrics.ca/).
*/

#ifndef CM_ESP32_MCP342X_H
#define CM_ESP32_MCP342X_H

#include <stdint.h>
#include <esp_system.h>
#include <esp_log.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*-----------------------------------------------------------
* MACROS & ENUMS
*----------------------------------------------------------*/

/**
 * I2C Address of device
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
    /** Write in OneShot Mode
     */
    MCP342X_CNTRL_NOTHING = 0x00,
    MCP342X_CNTRL_TRIGGER = 0x80,
    /** Reading from register
     */
    MCP342X_CNTRL_NOT_UPDATED = 0x80,
    MCP342X_CNTRL_UPDATED = 0x00,
    /** Mask to get Bit 7
     */
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
    MCP342X_SRATE_18BIT = 0x0C, // 0b 0000 1000
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

typedef enum MCP342xGeneralCalls
{
    MCP342X_GC_START = 0x00,
    MCP342X_GC_LATCH = 0x04,
    MCP342X_GC_RESET = 0x06,
    MCP342X_GC_CONVERSION = 0x08
} mcp342x_general_call_t;

/**
 * Configuration Register values of the MCP342x device
 * Initialized with default settings partially according to
 * datasheet Section 4.1
 */
typedef struct MCP342xConfig
{
    mcp342x_channel_t channel = MCP342X_CHANNEL_1;
    mcp342x_conversion_mode_t conversion_mode = MCP342X_MODE_ONESHOT;
    mcp342x_sample_rate_t sample_rate = MCP342X_SRATE_12BIT;
    mcp342x_gain_t gain = MCP342X_GAIN_1X;
} mcp342x_config_t;

/**
 * Struct ofr controlling a MCP342x device
 * smbus_info contains the i2c address of the device
 */
typedef struct MCP342xInfo
{
    bool init:1;
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
mcp342x_info_t *MCP342xMalloc(void);

/**
 * @brief Delete an existing MCP342x info instance.
 *
 * @param[in,out] Pointer to MCP342x info instance that will be freed and set to NULL.
 */
void MCP342xFree(mcp342x_info_t **mcp342x_info);

/**
 * @brief Initialise a MCP342x instance with the specified SMBus information.
 *
 * @param[in] mcp342x_info Pointer to MCP342x info instance.
 * @param[in] smbus_info Pointer to SMBus info instance.
 * @param[in] in_config Configuration bitmask. 
 * @return ESP_OK if successful, otherwise an error constant.
 */
esp_err_t MCP342xInit(mcp342x_info_t *mcp342x_info, smbus_info_t *smbus_info, mcp342x_config_t in_config);

/**
 * Specific call to the device samples the logic status 
 * of the Adr0 and Adr1 pins in the general call events
 */
esp_err_t MCP342xGeneralCall(const smbus_info_t *smbus_info_ptr, mcp342x_general_call_t call);

/**
 * @brief Trigger a conversion on the MCP342x instance
 *
 * @param[in] mcp342x_info Pointer to MCP342x info instance.
 * @return ESP_OK if successful, otherwise an error constant.
 */
esp_err_t MCP342xStartConversion(const mcp342x_info_t *mcp342x_info_ptr);

/**
 * @brief Read the result of the conversion
 *
 * @param[in] mcp342x_info_ptr Pointer to MCP342x info instance.
 * @param[in] smbus_info_ptr Pointer to SMBus info instance.
 * @param[in] i16_result_ptr Pointer to result info variable.
 * @return status_byte
 */
uint8_t MCP342xGetResult16(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int16_t *i16_result_ptr);

/**
 * @brief Read the result of the conversion
 *
 * @param[in] mcp342x_info_ptr Pointer to MCP342x info instance.
 * @param[in] smbus_info_ptr Pointer to SMbus info instace.
 * @param[in] i32_result_ptr Pointer to result info variable. 
 * @return status_byte
 */
uint8_t MCP342xGetResult18(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int32_t *i32_result_ptr);

/**
 * @brief Read the result of the conversion once
 *
 * @param[in] mcp342x_info_ptr Pointer to MCP342x info instance.
 * @param[in] smbus_info_ptr Pointer to SMBus info instance.
 * @param[in] i16_result_ptr Pointer to result info variable.
 * @return status_byte
 */
uint8_t MCP342xGetResultSingle16(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int16_t *i16_result_ptr);

/**
 * @brief Read the result of the conversion once
 *
 * @param[in] mcp342x_info_ptr Pointer to MCP342x info instance.
 * @param[in] smbus_info_ptr Pointer to SMbus info instace.
 * @param[in] i32_result_ptr Pointer to result info variable.
 * @return status_byte
 */
uint8_t MCP342xGetResultSingle18(const mcp342x_info_t *mcp342x_info_ptr, const smbus_info_t *smbus_info_ptr, int32_t *i32_result_ptr);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class MCP342x
{
  public:
    MCP342x(mcp342x_address_t in_address);
    void Configure(mcp342x_config_t in_config);
    uint8_t GetConfig(void);
    esp_err_t StartConversion();
  
  private:
    mcp342x_address_t address;
    uint8_t config;
};

#endif

#endif // CM_ESP32_MCP342X_H