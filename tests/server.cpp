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
            payloads.emplace_back(bytes.begin(), bytes.end());
            break;
        default:
            break;
        }

        lastMsg = Payload(bytes.begin(), bytes.end());
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


//TODO: check for bad connection
TEST_CASE("subscribe bytes") {
    const vector<ubyte> publish1
    {
        0x3c, 0x0d, //fixed header
        0x00, 0x05, 'f', 'i', 'r', 's', 't',//topic name
        0x00, 0x21, //message ID
        1, 2, 3, 4 //payload
    };

    const vector<ubyte> publish2
    {
        0x3c, 0x0d, //fixed header
        0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd',//topic name
        0x00, 0x21, //message ID
        9, 8, 7//payload
    };

    const vector<ubyte> publish3
    {
         0x3c, 0x0c, //fixed header
         0x00, 0x05, 't', 'h', 'i', 'r', 'd',//topic name
         0x00, 0x21, //message ID
         2, 4, 6, //payload
    };

    const vector<ubyte> subscribe
    {
         0x8b, 0x13, //fixed header
         0x33, 0x44, //message ID
         0x00, 0x05, 'f', 'i', 'r', 's', 't',
         0x01, //qos
         0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd',
         0x02, //qos
    };

    MqttServer<TestConnection> server;
    TestConnection connection;

    server.newMessage(connection, publish1);
    server.newMessage(connection, publish2);
    server.newMessage(connection, publish3);
    REQUIRE(connection.payloads == vector<Payload>{});

    server.newMessage(connection, subscribe);
    REQUIRE(connection.lastMsg == (vector<ubyte>{0x90, 3, 0x33, 0x44, 0}));
}
