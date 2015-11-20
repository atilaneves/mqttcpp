#ifndef CEREALISER_H_
#define CEREALISER_H_

#include "Cereal.hpp"
#include <type_traits>

class Cerealiser: public Cereal {
public:

    Cerealiser();

    virtual Type getType() const override { return Cereal::Type::Write; }

    template<typename T>
    Cerealiser& operator<<(const T& val) {
        Cereal::grain(const_cast<T&>(val)); //ok: grain doesn't modify anything
        return *this;
    }

    template<typename T>
    void write(const T& val) {
        *this << val;
    }

    template<typename I, typename T, typename A>
    void write(const std::vector<T, A>& vector) {
        Cereal::grain<I>(const_cast<std::vector<T, A>&>(vector)); //ok: grain doesn't modify anything
    }

    void writeBits(int value, int bits);

    //const Bytes& getBytes() const { return _bytes; }

private:

    uint8_t _currentByte;
    int _bitIndex;
    //Bytes _bytes;

    virtual void grainByte(uint8_t& val) override;
    virtual void grainBitsImpl(uint32_t& val, int bits) override { writeBits(val, bits); }
    virtual int bytesLeft() const override { return _bytes.size(); }
};

using Cerealizer = Cerealiser; //for people who can't spell :P


#endif // CEREALIN_H
