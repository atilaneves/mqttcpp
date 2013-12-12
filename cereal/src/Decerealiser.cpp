#include "Decerealiser.hpp"

void Decerealiser::grainByte(uint8_t& val) {
    val = *_iterator++;
}
