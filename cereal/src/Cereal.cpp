#include "Cereal.hpp"


Cereal::Cereal() {
}

Cereal::~Cereal() {
}

void Cereal::grain(bool& val) {
    grainReinterpret(val);
}

void Cereal::grain(uint8_t& val) {
    grainByte(val);
}

void Cereal::grain(int8_t& val) {
    grainReinterpret(val);
}

void Cereal::grain(char& val) {
    grainReinterpret(val);
}

void Cereal::grain(uint16_t& val) {
    uint8_t valh = (val >> 8);
    uint8_t vall = val & 0xff;
    grainByte(valh);
    grainByte(vall);
    val = (valh << 8) + vall;
}

void Cereal::grain(int16_t& val) {
    grainReinterpret(val);
}

void Cereal::grain(uint32_t& val) {
    uint8_t val0 = (val >> 24);
    uint8_t val1 = (val >> 16);
    uint8_t val2 = (val >> 8);
    uint8_t val3 = val & 0xff;
    grainByte(val0);
    grainByte(val1);
    grainByte(val2);
    grainByte(val3);
    val = (val0 << 24) + (val1 << 16) + (val2 << 8) + val3;
}

void Cereal::grain(int32_t& val) {
    grainReinterpret(val);
}

void Cereal::grain(uint64_t& val) {
    const auto ptr = reinterpret_cast<uint8_t*>(&val);
    for(int i = 7; i >= 0; --i) {
        grainByte(ptr[i]);
    }
    uint64_t newVal = 0;
    for(int i = 7; i >= 0; --i) {
        newVal += (ptr[i] << (i * 8));
    }
}

void Cereal::grain(int64_t& val) {
    grainReinterpret(val);
}


void Cereal::grain(double& val) {
    grainReinterpret(val);
}
