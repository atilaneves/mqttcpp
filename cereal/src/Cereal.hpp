#ifndef CEREAL_HPP_
#define CEREAL_HPP_

#include "CerealTraits.hpp"
#include <vector>
#include <stdint.h>
#include <algorithm>


class Cereal {
public:

    enum class Type { Write, Read };

    using Byte = unsigned char;
    using Bytes = std::vector<Byte>;

    Cereal(const Cereal&) = delete;
    Cereal& operator=(const Cereal&) = delete;
    virtual ~Cereal();

    virtual Type getType() const = 0;

    void grain(bool& val);
    void grain(uint8_t& val);
    void grain(int8_t& val);
    void grain(uint16_t& val);
    void grain(int16_t& val);
    void grain(uint32_t& val);
    void grain(int32_t& val);
    void grain(uint64_t& val);
    void grain(int64_t& val);
    void grain(double& val);

    template<typename I, typename T, typename A>
    void grain(std::vector<T, A>& vector);

    template<typename T>
    void grain(T& t) {
        t.cerealise(*this);
    }

    const Bytes& getBytes() const { return _bytes; }

protected:

    Bytes _bytes;

    Cereal();
    template<typename T> Cereal(const T& bytes):
        _bytes(std::begin(bytes), std::end(bytes)) { }

private:

    template<typename T>
    void grainReinterpret(T& val) {
        const auto uptr = reinterpret_cast<typename CerealTraits::MakeUnsigned<T>::Type*>(&val);
        grain(*uptr);
    }

    virtual void grainByte(uint8_t& val) = 0;
};

template<typename I, typename V>
static void maybeResizeVector(Cereal& cereal, V& vec) {
    I num;
    cereal.grain(num);
    if(vec.size() != num) { //writing to vector, resize it
        vec.resize(num);
    }
}

template<typename I, typename T, typename A>
void Cereal::grain(std::vector<T, A>& vector) {
    I num = vector.size();
    grain(num);
    if(vector.size() != num) { //writing to vector, resize it
        vector.resize(num);
    }
    for(auto& t: vector) {
        t.cerealise(*this);
    }
}


#endif // CEREAL_HPP_
