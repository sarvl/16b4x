#include <cstdint>

uint16_t register_fl_read();
void     register_fl_write(uint16_t const val);
void     modify_flags(uint32_t const res, uint32_t const A, uint32_t const B);
