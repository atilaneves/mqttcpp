#include "unit_thread.hpp"
#include <cereal_all>
#include <limits>


struct TestInU8: public TestCase {
    virtual void test() override final {
        std::vector<uint8_t> ins{4, 5, 6};
        Cerealiser cerealiser;
        for(const auto in: ins) cerealiser << in;
        checkEqual(cerealiser.getBytes(), ins);
    }
};
REGISTER_TEST(8, TestInU8)


template<typename T>
struct TestInOut: public TestCase {
    std::vector<T> _ins;
    TestInOut(const std::vector<T>& ins):_ins(ins) { }

    virtual void test() override {
        Cerealiser cerealiser;
        for(const auto in: _ins) cerealiser << in;

        Decerealiser decerealiser(cerealiser.getBytes());
        std::vector<T> outs(_ins.size());
        for(auto& out: outs) decerealiser >> out;

        checkEqual(outs, _ins);
    }
};


struct TestInOutU8: public TestInOut<uint8_t> {
    TestInOutU8():TestInOut<uint8_t>({2, 5, 7, 3}) { }
};
REGISTER_TEST(8, TestInOutU8)


struct TestInOutS8: public TestInOut<int8_t> {
    TestInOutS8():TestInOut<int8_t>({-2, -5, 7, 3, -9}) { }
};
REGISTER_TEST(8, TestInOutS8)


struct TestInOutU16: public TestInOut<uint16_t> {
    TestInOutU16():TestInOut<uint16_t>({2, 65000, 7, 0xffff}) { }
};
REGISTER_TEST(16, TestInOutU16)


struct TestInOutS16: public TestInOut<int16_t> {
    TestInOutS16():TestInOut<int16_t>({-2, -32000, 32000, 3, -9}) { }
};
REGISTER_TEST(16, TestInOutS16)

struct TestInOutU32: public TestInOut<uint32_t> {
    TestInOutU32():TestInOut<uint32_t>({std::numeric_limits<uint32_t>::max(), 5, 7, 3}) { }
};
REGISTER_TEST(32, TestInOutU32)


struct TestInOutS32: public TestInOut<int32_t> {
    TestInOutS32():TestInOut<int32_t>({-std::numeric_limits<uint32_t>::max(), -5, 7, 3, -1}) { }
};
REGISTER_TEST(32, TestInOutS32)

struct TestInOutS64: public TestInOut<int64_t> {
    TestInOutS64():TestInOut<int64_t>({-std::numeric_limits<uint64_t>::max(),
                static_cast<int64_t>(std::numeric_limits<uint32_t>::max()) + 1}) { }
};
REGISTER_TEST(64, TestInOutS64)


struct Double: public TestInOut<double> {
    Double():TestInOut<double>({1.0, -2.0, 3456789.0}) { }
};
REGISTER_TEST(nonints, Double)


struct Foo {
    uint16_t i16;
    int32_t i32;
    double d;
    Foo():i16(), i32(), d() { }
    Foo(uint16_t _i16, int32_t _i32, double _d):i16(_i16), i32(_i32), d(_d) { }
    bool operator==(const Foo& f) const {
        return i16 == f.i16 && i32 == f.i32 && d == f.d;
    }
    void cerealise(Cereal& cereal) {
        cereal.grain(i16);
        cereal.grain(i32);
        cereal.grain(d);
    }
};

std::ostream& operator<<(std::ostream& o, const std::vector<Foo>& foos) {
    for(const auto& foo: foos)
        o << "Foo{i16: " <<  std::to_string(foo.i16) <<
            ", i32: " << std::to_string(foo.i32) << ", d: " << std::to_string(foo.d) << "} ";
    return o;
}

struct FooScalar: public TestInOut<Foo> {
    FooScalar():TestInOut<Foo>({{1, 2, 3.0}, {2, -4, 6.0}}) { }
};
REGISTER_TEST(scalar, FooScalar)

struct FooVector: public TestCase {
    virtual void test() override {
        const std::vector<Foo> foos{{0xffff, 79999, 3.0}, {3, -99999, 4.0}};
        Cerealiser cerealiser;
        cerealiser.write<uint8_t>(foos);
        checkEqual(foos.size(), 2);

        Decerealiser decerealiser(cerealiser.getBytes());
        std::vector<Foo> outs;
        decerealiser.grain<uint8_t>(outs);

        checkEqual(outs.size(), foos.size());
        checkEqual(outs, foos);
    }
};
REGISTER_TEST(vector, FooVector)
