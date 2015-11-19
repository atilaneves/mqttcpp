#include "catch.hpp"
#include "stream.hpp"
#include "gsl.h"
#include <sstream>


using namespace std;
using namespace gsl;

using Payload = vector<ubyte>;
struct TestConnection {
    void newMessage(span<const ubyte> bytes) {
        switch(getMessageType(bytes)) {
        case MqttType::CONNACK:
            connected = true;
            connectionCode = MqttConnack::Code::ACCEPTED;
            break;

        case MqttType::PUBLISH:
            {
                Decerealiser dec{bytes};
                const auto hdr = dec.create<MqttFixedHeader>();
                dec.reset();
                const auto msg = dec.create<MqttPublish>(hdr);
                payloads.emplace_back(msg.payload.begin(), msg.payload.end());
            }
            break;
        default:
            break;
        }

        lastMsg = Payload(bytes.begin(), bytes.end());
    }

    void disconnect() {
        connected = false;
    }

    bool connected{false};
    MqttConnack::Code connectionCode{MqttConnack::Code::NO_AUTH};
    vector<Payload> payloads;
    Payload lastMsg;
};

static vector<ubyte> subscribeMsg(const std::string& topic, ushort msgId) {
    vector<ubyte> msg{0x8b}; //fixed header sans remaining length
    const auto remainingLength = topic.size() + 2 /*topic len*/ + 2 /*msgId*/ + 1 /*qos*/;
    msg.emplace_back(remainingLength);

    msg.emplace_back(msgId >> 8);
    msg.emplace_back(msgId & 0xff);

    msg.emplace_back(topic.size() >> 8);
    msg.emplace_back(topic.size() & 0xff);
    copy(topic.cbegin(), topic.cend(), back_inserter(msg));

    msg.emplace_back(0); //qos

    return msg;
}


TEST_CASE("MQTT in 2 packets") {
    MqttServer<TestConnection> server;
    TestConnection connection;
    MqttStream stream{128};

    const auto subscribe = subscribeMsg("top", 33);
    server.newMessage(connection, subscribe);

    const vector<ubyte> bytes1{
        0x3c, 0x0f, //fixed header
        0x00, 0x03, 't', 'o', 'p', //topic name
        0x00, 0x21, //message ID
        1, 2, 3 //first part of payload
    };

    stream << bytes1;
    stream.handleMessages(server, connection);
    REQUIRE(connection.payloads == vector<Payload>{});

    const vector<ubyte> bytes2{4, 5, 6, 7, 8, 9}; //2nd part of payload
    stream << bytes2;
    stream.handleMessages(server, connection);
    REQUIRE(connection.payloads == (vector<Payload>{{1, 2, 3, 4, 5, 6, 7, 8, 9}}));
}
