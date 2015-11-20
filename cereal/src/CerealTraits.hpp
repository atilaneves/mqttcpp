#ifndef CEREALTRAITS_H_
#define CEREALTRAITS_H_


#include <type_traits>
#include <stdint.h>

namespace CerealTraits {
    template<typename T>
    struct MakeUnsigned {
        using Type = typename std::make_unsigned<T>::type;
    };

    template<>
    struct MakeUnsigned<bool> {
        using Type = uint8_t;
    };

    template<>
    struct MakeUnsigned<double> {
        using Type = uint64_t;
    };
}


#endif // CEREALTRAITS_H_
