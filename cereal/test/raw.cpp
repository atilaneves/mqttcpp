#include "unit_thread.hpp"
#include <cereal_all>
#include <limits>

struct EncodeRawArray: public TestCase {
    void test() override {
        Cerealiser cereal;
        std::vector<uint8_t> data{1, 2, 3};
        cereal.grainRawArray(data);
        checkEqual(cereal.getBytes(), data);
    }
};
REGISTER_TEST(raw, EncodeRawArray)


struct DecodeRawArray: public TestCase {
    void test() override {
        std::vector<uint8_t> data{1, 2, 3};
        Decerealiser cereal(data);
        std::vector<uint8_t> result;
        cereal.grainRawArray(result);
        checkEqual(result, data);
    }
};
REGISTER_TEST(raw, DecodeRawArray)
