#include "unit_thread.hpp"
#include <cereal_all>
#include <iostream>


namespace {
struct Foo {
    Foo():i1(), i2(), s()  {}
    void cerealise(Cereal& cereal) {
        cereal.grain(i1);
        cereal.grain(i2);
        cereal.grain(s);
    }
    uint8_t i1;
    uint8_t i2;
    std::string s;
};
} //anonymous namespace


struct EncodeClass: public TestCase {
    void test() override {
        Decerealiser cereal(std::vector<uint8_t>{1, 4, 0, 3, 'f', 'o', 'o'});
        auto foo = cereal.createPtr<Foo>();
        checkEqual(foo->i1, 1);
        checkEqual(foo->i2, 4);
        checkEqual(foo->s, std::string("foo"));
    }
};
REGISTER_TEST(create, EncodeClass)
