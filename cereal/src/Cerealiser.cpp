#include "Cerealiser.hpp"


Cerealiser::Cerealiser():_currentByte(0), _bitIndex(0) {

}

void Cerealiser::grainByte(uint8_t& val) {
    _bytes.push_back(val);
}

void Cerealiser::writeBits(int value, int bits) {
    constexpr int bitsInByte = 8;
    if(_bitIndex + bits >= bitsInByte) { //carries over to next byte
        const auto remainingBits = _bitIndex + bits - bitsInByte;
        const auto thisByteValue = (value >> remainingBits);
        _currentByte |= thisByteValue;
        *this << _currentByte;
        _currentByte = 0;
        _bitIndex = 0;
        if(remainingBits > 0) {
            uint8_t remainingValue = value & (0xff >> (bitsInByte - remainingBits));
            writeBits(remainingValue, remainingBits);
            return;
        }
    }
    _currentByte |= (value << (bitsInByte - bits - _bitIndex));
    _bitIndex += bits;
}
