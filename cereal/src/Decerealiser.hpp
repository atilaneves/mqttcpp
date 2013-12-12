#ifndef DECEREALISER_H_
#define DECEREALISER_H_


#include "Cereal.hpp"


class Decerealiser: public Cereal {
public:

    template<typename T> Decerealiser(const T& bytes):
        Cereal(bytes), _iterator(std::begin(bytes)) { }

    template<typename T>
    Decerealiser& operator>>(T& val) {
        Cereal::grain(val);
        return *this;
    }

    template<typename T>
    T read() {
        T val;
        *this >> val;
        return val;
    }

private:

    Bytes::const_iterator _iterator;

    virtual void grainByte(uint8_t& val) override;

};


using Decerealizer = Decerealiser; //for people who can't spell :P

#endif // DECEREALISER_H
