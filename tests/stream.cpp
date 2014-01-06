#include "unit_thread.hpp"
#include "stream.hpp"


struct MqttInTwoPackets: public TestCase {
    virtual void test() override {
        std::vector<ubyte> bytes1{ 0x3c, 0x0f, //fixed header
                0x00, 0x03, 't', 'o', 'p', //topic name
                0x00, 0x21, //message ID
                'b', 'o', 'r' }; //1st part of payload
        MqttStream stream(128);
        stream << bytes1;
        checkFalse(stream.hasMessages());
        checkNull(stream.createMessage().get());

        std::vector<ubyte> bytes2{ 'o', 'r', 'o', 'o', 'n'}; //2nd part of payload
        stream << bytes2;
        checkTrue(stream.hasMessages());
        const auto publish = dynamic_cast<MqttPublish*>(stream.createMessage().get());
        checkNotNull(publish);
    }
};
REGISTER_TEST(stream, MqttInTwoPackets)


struct TwoMqttInThreePackets: public TestCase {
    virtual void test() override {
        std::vector<ubyte> bytes1{ 0x3c, 0x0f, //fixed header
                0x00, 0x03, 't', 'o', 'p', //topic name
                0x00, 0x21, //message ID
                'a', 'b', 'c' }; //1st part of payload
        MqttStream stream(128);
        stream << bytes1;
        checkFalse(stream.hasMessages());
        checkNull(stream.createMessage().get());
        checkFalse(stream.empty());

        std::vector<ubyte> bytes2{ 'd', 'e', 'f', 'g', 'h'}; //2nd part of payload
        stream << bytes2;
        checkTrue(stream.hasMessages());
        const auto publish = dynamic_cast<MqttPublish*>(stream.createMessage().get());
        checkNotNull(publish);
        checkTrue(stream.empty());

        std::vector<ubyte> bytes3{0xe0, 0x00};
        stream << bytes3;
        checkFalse(stream.empty());
        const auto disconnect = dynamic_cast<MqttDisconnect*>(stream.createMessage().get());
        checkNotNull(disconnect);
        checkTrue(stream.empty());
    }
};
REGISTER_TEST(stream, TwoMqttInThreePackets)


struct TwoMqttInOnePacket: public TestCase {
    virtual void test() override {
        MqttStream stream(128);
        checkFalse(stream.hasMessages());
        checkTrue(stream.empty());

        std::vector<ubyte> bytes1{ 0x3c }; // half of header
        std::vector<ubyte> bytes2{ 0x0f, //2nd half fixed header
                0x00, 0x03, 't', 'o', 'p', //topic name
                0x00, 0x21, //message ID
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', //payload
                0xe0, 0x00, //header for disconnect
                };
        stream << bytes1;
        checkFalse(stream.empty());
        checkFalse(stream.hasMessages());

        stream << bytes2;
        checkFalse(stream.empty());
        checkTrue(stream.hasMessages());

        const auto publish = dynamic_cast<MqttPublish*>(stream.createMessage().get());
        checkNotNull(publish);
        checkFalse(stream.empty());
        checkTrue(stream.hasMessages());

        const auto disconnect = dynamic_cast<MqttDisconnect*>(stream.createMessage().get());
        checkNotNull(disconnect);
        checkTrue(stream.empty());
    }
};
REGISTER_TEST(stream, TwoMqttInOnePacket)
