#ifndef VALVEIO_HPP
#define VALVEIO_HPP

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#ifdef __cplusplus
}
#endif

namespace valveio
{

#define VALVE_IO_ADDR  0x20

void init();
void set(uint8_t x, bool y);
void reset();

}

#endif
