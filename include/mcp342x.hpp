#ifndef ESP32_MCP342X_HPP
#define ESP32_MCP342X_HPP

#ifdef __cplusplus
extern "C"
{
#endif

#include "mcp342x.h"

#ifdef __cplusplus
}
#endif

namespace cm
{

class MCP342x
{
  public:
    MCP342x(mcp342x_address_t in_address);
    ~MCP342x();
    esp_err_t Init(i2c_port_t in_i2c_master, mcp342x_config_t in_config);
    esp_err_t SetConfig(mcp342x_config_t in_config);
    esp_err_t StartConversion(void);
    uint8_t GetResult();
    uint8_t GetResultSingle();
    mcp342x_address_t GetAddress(void);
    mcp342x_info_t *GetInfoPtr(void);

  private:
    mcp342x_address_t address;
    mcp342x_info_t *mcp342x_info;
    int32_t result;
};

} // namespace cm

#endif // ESP32_MCP342X_HPP
