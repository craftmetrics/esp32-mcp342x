#include "valveio.hpp"
#include "smbus.h"

namespace valveio
{

static smbus_info_t * valveIO;
static uint8_t valveio_state = 0xff;

void init() {
    valveIO = smbus_malloc();
    smbus_init(valveIO, I2C_NUM_0, VALVE_IO_ADDR);
    smbus_set_timeout(valveIO, 1000 / portTICK_RATE_MS);
}

void set(uint8_t x, bool y) {
    valveio_state = (~(1<<x) & valveio_state) | (y&1)<<x;
    smbus_send_byte(valveIO, valveio_state);
}
void reset() {
    valveio_state = 0xff;
    smbus_send_byte(valveIO, valveio_state);
}

} // valveio
