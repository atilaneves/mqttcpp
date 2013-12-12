#include "Cerealiser.hpp"


void Cerealiser::grainByte(uint8_t& val) {
    _bytes.push_back(val);
}
