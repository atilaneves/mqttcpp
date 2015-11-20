#include "Decerealiser.hpp"


Decerealiser::Decerealiser():_currentByte(0), _bitIndex(0) {

}

void Decerealiser::grainByte(uint8_t& val) {
    val = *_iterator++;
}

uint32_t Decerealiser::readBits(int bits) {
    if(_bitIndex == 0) {
        _currentByte = read<uint8_t>();
    }

    return readBitsHelper(bits);
}

uint32_t Decerealiser::readBitsHelper(int bits) {
    constexpr int bitsInByte = 8;
    if(_bitIndex + bits > bitsInByte) { //have to carry on to the next byte
        const auto bits1stTime = bitsInByte - _bitIndex; //what's left of this byte
        const auto bits2ndTime = (_bitIndex + bits) - bitsInByte; //bits to read from next byte
        const auto value1 = readBitsHelper(bits1stTime);
        _bitIndex = 0;
        _currentByte = read<uint8_t>();
        const auto value2 = readBitsHelper(bits2ndTime);
        return (value1 << bits2ndTime) | value2;
    }

    _bitIndex += bits;

    auto shift = _currentByte >> (bitsInByte - _bitIndex);
    return shift & (0xff >> (bitsInByte - bits));
}


void Decerealiser::reset() {
    /**resets the deceraliser to read from the beginning again*/
    _bitIndex = 0;
    _currentByte = 0;
    _iterator = _originalIterator;
}
