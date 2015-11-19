#include "catch.hpp"
#include "server.hpp"
#include "gsl.h"

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

static vector<ubyte> connectionMsgBytes() {
    return {0x10, 0x2a, //fixed header
            0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', //protocol name
            0x03, //protocol version
            0xcc, //connection flags 1100111x username, pw, !wr, w(01), w, !c, x
            0x00, 0x0a, //keepalive of 10
            0x00, 0x03, 'c', 'i', 'd', //client ID
            0x00, 0x04, 'w', 'i', 'l', 'l', //will topic
            0x00, 0x04, 'w', 'm', 's', 'g', //will msg
            0x00, 0x07, 'g', 'l', 'i', 'f', 't', 'e', 'l', //username
            0x00, 0x02, 'p', 'w', //password
            };
}

TEST_CASE("connect") {
    MqttServer<TestConnection> server;
    TestConnection connection;

    const auto bytes = connectionMsgBytes();
    server.newMessage(connection, bytes);
    REQUIRE(connection.connected == true);
    REQUIRE(connection.connectionCode == MqttConnack::Code::ACCEPTED);
}


vector<ubyte> publishMsg(const std::string& topic, ushort msgId, initializer_list<ubyte> payload) {
    vector<ubyte> msg{0x3c}; //fixed header sans remaining length
    const auto remainingLength = topic.size() + 2 /*topic len*/+ 2 /*msgIdLen*/ + payload.size();
    msg.emplace_back(remainingLength); //not strictly correct, but ok for testing

    msg.emplace_back(topic.size() >> 8);
    msg.emplace_back(topic.size() & 0xff);
    copy(topic.cbegin(), topic.cend(), back_inserter(msg));

    msg.emplace_back(msgId >> 8);
    msg.emplace_back(msgId & 0xff);

    copy(payload.begin(), payload.end(), back_inserter(msg));
    return msg;
}

TEST_CASE("publishMsg") {
    REQUIRE(publishMsg("third", 0x4321, {2, 4, 6}) ==
            (vector<ubyte>{
                0x3c, 0x0c, //fixed header
                0x00, 0x05, 't', 'h', 'i', 'r', 'd',//topic name
                0x43, 0x21, //message ID (network byte order)
                2, 4, 6, //payload
            }));
}

vector<ubyte> subscribeMsgBytes() {
    return
    {
        0x8b, 0x13, //fixed header
        0x33, 0x44, //message ID
        0x00, 0x05, 'f', 'i', 'r', 's', 't',
        0x01, //qos
        0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd',
        0x02, //qos
    };
}

//TODO: check for bad connection
TEST_CASE("subscribe bytes") {
    const auto publish1 = publishMsg("first", 0x21, {1, 2, 3, 4});
    const auto publish2 = publishMsg("second", 0x33, {9, 8, 7});
    const auto publish3 = publishMsg("third", 0x44, {2, 4, 6});
    const auto subscribe = subscribeMsgBytes();

    MqttServer<TestConnection> server;
    TestConnection connection;

    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == vector<Payload>{});

    server.newMessage(connection, subscribe);
    REQUIRE(connection.lastMsg == (vector<ubyte>{0x90, 3, 0x33, 0x44, 0}));

    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == (vector<Payload>{{1, 2, 3, 4}, {9, 8, 7}}));
}

TEST_CASE("ping bytes") {
    MqttServer<TestConnection> server;
    TestConnection connection;

    const vector<ubyte> ping{0xc0, 0};
    server.newMessage(connection, ping);
    REQUIRE(connection.lastMsg == (vector<ubyte>{0xd0, 0}));
}


TEST_CASE("unsubscribe topic bytes") {
    const auto publish1 = publishMsg("first", 0x21, {1, 2, 3, 4});
    const auto publish2 = publishMsg("second", 0x33, {9, 8, 7});
    const auto publish3 = publishMsg("third", 0x44, {2, 4, 6});
    const auto subscribe = subscribeMsgBytes();

    const vector<ubyte> unsubscribe1
    {
        0xa2, 10, //fixed header
        0x43, 0x21, //msg id
        0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd' // topic
    };

    MqttServer<TestConnection> server;
    TestConnection connection;

    server.newMessage(connection, subscribe);
    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == (vector<Payload>{{1, 2, 3, 4}, {9, 8, 7}}));

    server.newMessage(connection, unsubscribe1);
    //unsuback
    REQUIRE(connection.lastMsg == (vector<ubyte>{0xb0, 2, 0x43, 0x21}));

    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == (vector<Payload>{{1, 2, 3, 4}, {9, 8, 7}, {1, 2, 3, 4}}));

    const vector<ubyte> unsubscribe2{
        0xa2, 9, //fixed header
        0x12, 0x34, //msg id
        0x00, 0x05, 'f', 'i', 'r', 's', 't',
    };

    //unsuback
    server.newMessage(connection, unsubscribe2);
    REQUIRE(connection.lastMsg == (vector<ubyte>{0xb0, 2, 0x12, 0x34}));

    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == (vector<Payload>{{1, 2, 3, 4}, {9, 8, 7}, {1, 2, 3, 4}}));

}

TEST_CASE("unsubscribe all bytes") {
    const auto publish1 = publishMsg("first", 0x21, {1, 2, 3, 4});
    const auto publish2 = publishMsg("second", 0x33, {9, 8, 7});
    const auto publish3 = publishMsg("third", 0x44, {2, 4, 6});
    const auto subscribe = subscribeMsgBytes();
    const vector<ubyte> disconnect{0xe0, 0};

    MqttServer<TestConnection> server;
    TestConnection connection;

    const auto bytes = connectionMsgBytes();
    server.newMessage(connection, bytes);
    REQUIRE(connection.connected == true);

    server.newMessage(connection, subscribe);
    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == (vector<Payload>{{1, 2, 3, 4}, {9, 8, 7}}));

    server.newMessage(connection, disconnect);
    REQUIRE(connection.connected == false);

    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == (vector<Payload>{{1, 2, 3, 4}, {9, 8, 7}}));
}
