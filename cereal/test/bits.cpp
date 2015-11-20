#include "unit_thread.hpp"
#include <cereal_all>


struct EncodeNibble: public TestCase {
    void test() override {
        Cerealiser cereal;
        cereal.writeBits(0x4, 4);
        cereal.writeBits(0xf, 4);
        checkEqual(cereal.getBytes(), std::vector<uint8_t>({0x4f}));
    }
};
REGISTER_TEST(bits/encode, EncodeNibble)


struct EncodeSubByte: public TestCase {
    void test() override {
        Cerealiser cereal;
        cereal.writeBits(1, 1);
        cereal.writeBits(3, 2);
        cereal.writeBits(0, 1);
        cereal.writeBits(5, 3);
        cereal.writeBits(1, 1);
        checkEqual(cereal.getBytes(), std::vector<uint8_t>({0xeb}));
    }
};
REGISTER_TEST(bits/encode, EncodeSubByte)


struct EncodeSubWord: public TestCase {
    void test() override {
        Cerealiser cereal;
        cereal.writeBits(4, 3);
        cereal.writeBits(7, 3);
        cereal.writeBits(23, 5);
        cereal.writeBits(1, 2);
        cereal.writeBits(2, 3);
        checkEqual(cereal.getBytes(), std::vector<uint8_t>({0x9e, 0xea}));
    }
};
REGISTER_TEST(bits/encode, EncodeSubWord)


struct EncodeMoreThan8Bits: public TestCase {
    void test() override {
        {
            Cerealiser cereal;
            cereal.writeBits(1, 9);
            cereal.writeBits(15, 7);
            checkEqual(cereal.getBytes(), std::vector<uint8_t>({0x00, 0x8f}));
        }
        {
            Cerealiser cereal;
            cereal.writeBits((0x9e << 1) | 1, 9);
            cereal.writeBits(0xea & 0x7f, 7);
            checkEqual(cereal.getBytes(), std::vector<uint8_t>({0x9e, 0xea}));
        }
    }
};
REGISTER_TEST(bits/encode, EncodeMoreThan8Bits)


struct DecodeBits: public TestCase {
    void test() override {
        Decerealiser cereal(std::vector<uint8_t>{0x9e, 0xea});
        //1001 1110 1110 1010 or
        //100 111 10111 01 010
        //checkEqual(cereal.readBits(3), 4);
        auto foo = cereal.readBits(3);
        checkEqual(foo, 4);
        checkEqual(cereal.readBits(3), 7);
        checkEqual(cereal.readBits(5), 23);
        checkEqual(cereal.readBits(2), 1);
        checkEqual(cereal.readBits(3), 2);

        cereal.reset();
        checkEqual(cereal.readBits(3), 4);
        checkEqual(cereal.readBits(3), 7);
        checkEqual(cereal.readBits(5), 23);
        checkEqual(cereal.readBits(2), 1);
        checkEqual(cereal.readBits(3), 2);
    }
};
REGISTER_TEST(bits/decode, DecodeBits)



struct DecodeBitsMultiByte: public TestCase {
    void test() override {
        Decerealiser cereal(std::vector<uint8_t>{0x9e, 0xea});
        checkEqual(cereal.readBits(9), 317);
        checkEqual(cereal.readBits(7), 0x6a);
    }
};
REGISTER_TEST(bits/decode, DecodeBitsMultiByte)


struct DecodeBitsIter: public TestCase {
    void test() override {
        const auto bytes = std::vector<uint8_t>{0x9e, 0xea};
        Decerealiser cereal(bytes.cbegin(), bytes.cend());
        //1001 1110 1110 1010 or
        //100 111 10111 01 010
        //checkEqual(cereal.readBits(3), 4);
        auto foo = cereal.readBits(3);
        checkEqual(foo, 4);
        checkEqual(cereal.readBits(3), 7);
        checkEqual(cereal.readBits(5), 23);
        checkEqual(cereal.readBits(2), 1);
        checkEqual(cereal.readBits(3), 2);

        cereal.reset();
        checkEqual(cereal.readBits(3), 4);
        checkEqual(cereal.readBits(3), 7);
        checkEqual(cereal.readBits(5), 23);
        checkEqual(cereal.readBits(2), 1);
        checkEqual(cereal.readBits(3), 2);
    }
};
REGISTER_TEST(bits/decode, DecodeBitsIter)
