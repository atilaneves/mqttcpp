#include "unit_thread.hpp"
#include "server.hpp"
#include "factory.hpp"


struct TestMqttConnection: public MqttConnection {
    TestMqttConnection(std::vector<ubyte> bytes):
        connected(false),
        connect(dynamic_cast<MqttConnect*>(MqttFactory::create(bytes).release())) {
    }

    TestMqttConnection(std::unique_ptr<MqttConnect>&& c):
        connected(true), connect(std::move(c)) {
    }

    void write(const std::vector<ubyte>& bytes) override {
        lastMsg = MqttFactory::create(bytes);
    }

    void newMessage(std::string topic, const std::vector<ubyte>& payload) override {
        (void)topic;
        payloads.emplace_back(payload.begin(), payload.end());
    }

    void disconnect() override { connected = false; }

    std::unique_ptr<MqttMessage> lastMsg;
    std::vector<std::string> payloads;
    bool connected;
    std::unique_ptr<MqttConnect> connect;
};


struct Connect: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x2a, //fixed header
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

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());
        const auto connack = dynamic_cast<MqttConnack*>(connection.lastMsg.get());
        checkNotNull(connack);
        checkEqual((int)connack->code, (int)MqttConnack::Code::ACCEPTED);
    }
};
REGISTER_TEST(server, Connect)

struct ConnectBigId: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x3f, //fixed header
                0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', //protocol name
                0x03, //protocol version
                0xcc, //connection flags 1100111x username, pw, !wr, w(01), w, !c, x
                0x00, 0x0a, //keepalive of 10
                0x00, 0x18, 'c', 'i', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd',
                'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd', //24 char client id
                0x00, 0x04, 'w', 'i', 'l', 'l', //will topic
                0x00, 0x04, 'w', 'm', 's', 'g', //will msg
                0x00, 0x07, 'g', 'l', 'i', 'f', 't', 'e', 'l', //username
                0x00, 0x02, 'p', 'w', //password
                };

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());
        const auto connack = dynamic_cast<MqttConnack*>(connection.lastMsg.get());
        checkNotNull(connack);
        checkEqual((int)connack->code, (int)MqttConnack::Code::BAD_ID);
    }
};
REGISTER_TEST(server, ConnectBigId)

struct ConnectSmallId: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x27, //fixed header
                0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', //protocol name
                0x03, //protocol version
                0xcc, //connection flags 1100111x username, pw, !wr, w(01), w, !c, x
                0x00, 0x0a, //keepalive of 10
                0x00, 0x00, //no client id
                0x00, 0x04, 'w', 'i', 'l', 'l', //will topic
                0x00, 0x04, 'w', 'm', 's', 'g', //will msg
                0x00, 0x07, 'g', 'l', 'i', 'f', 't', 'e', 'l', //username
                0x00, 0x02, 'p', 'w', //password
                };

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());
        const auto connack = dynamic_cast<MqttConnack*>(connection.lastMsg.get());
        checkNotNull(connack);
        checkEqual((int)connack->code, (int)MqttConnack::Code::BAD_ID);
    }
};
REGISTER_TEST(server, ConnectSmallId)

struct Subscribe: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x2a, //fixed header
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

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());

        server.publish("foo/bar/baz", "interesting stuff");
        checkEqual(connection.payloads, std::vector<std::string>({}));

        server.subscribe(connection, 42, std::vector<std::string>({"foo/bar/+"}));
        const auto suback = dynamic_cast<MqttSuback*>(connection.lastMsg.get());
        checkNotNull(suback);
        checkEqual(suback->msgId, 42);
        checkEqual(suback->qos, std::vector<ubyte>({0}));

        server.publish("foo/bar/baz", "interesting stuff");
        server.publish("foo/boogagoo", "oh noes!!!");
        checkEqual(connection.payloads, std::vector<std::string>({"interesting stuff"}));

        server.unsubscribe(connection);
        server.publish("foo/bar/baz", "interesting stuff");
        server.publish("foo/boogagoo", "oh noes!!!");
        checkEqual(connection.payloads,
                   std::vector<std::string>({"interesting stuff"})); //shouldn't have changed
    }
};
REGISTER_TEST(server, Subscribe)


struct SubscribeWithMessage: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x2a, //fixed header
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

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());

        server.publish("foo/bar/baz", "interesting stuff");
        checkEqual(connection.payloads, std::vector<std::string>({}));

        bytes = std::vector<ubyte>{ 0x8c, 0x13, //fixed header
                                    0x00, 0x21, //message ID
                                    0x00, 0x05, 'f', 'i', 'r', 's', 't',
                                    0x01, //qos
                                    0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd',
                                    0x02, //qos
        };

        const auto msg = MqttFactory::create(bytes);
        checkNotNull(msg.get());
        msg->handle(server, connection); //subscribe
        const auto suback = dynamic_cast<MqttSuback*>(connection.lastMsg.get());
        checkNotNull(suback);
        checkEqual(suback->msgId, 33);
        checkEqual(suback->qos, std::vector<ubyte>({1, 2}));

        bytes ={ 0x3c, 0x0d, //fixed header
                0x00, 0x05, 'f', 'i', 'r', 's', 't',//topic name
                0x00, 0x21, //message ID
                'b', 'o', 'r', 'g', //payload
                };
        MqttFactory::create(bytes)->handle(server, connection); //publish

        bytes = { 0x3c, 0x0d, //fixed header
                0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd',//topic name
                0x00, 0x21, //message ID
                'f', 'o', 'o',//payload
                };
        MqttFactory::create(bytes)->handle(server, connection); //publish

        bytes = { 0x3c, 0x0c, //fixed header
                0x00, 0x05, 't', 'h', 'i', 'r', 'd',//topic name
                0x00, 0x21, //message ID
                'f', 'o', 'o',//payload
                };
        MqttFactory::create(bytes)->handle(server, connection); //publish


        checkEqual(connection.payloads, std::vector<std::string>({"borg", "foo"}));
    }
};
REGISTER_TEST(server, SubscribeWithMessage)

struct Unsubscribe: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x2a, //fixed header
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

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());

        server.subscribe(connection, 42, std::vector<std::string>({"foo/bar/+"}));
        const auto suback = dynamic_cast<MqttSuback*>(connection.lastMsg.get());
        checkNotNull(suback);

        server.publish("foo/bar/baz", "interesting stuff");
        server.publish("foo/boogagoo", "oh noes!!!");
        checkEqual(connection.payloads, std::vector<std::string>({"interesting stuff"}));

        server.unsubscribe(connection, 2, std::vector<std::string>({"boo"})); //doesn't exist, so no effect
        const auto unsuback1 = dynamic_cast<MqttUnsuback*>(connection.lastMsg.get());
        checkNotNull(unsuback1);
        checkEqual(unsuback1->msgId, 2);

        server.publish("foo/bar/baz", "interesting stuff");
        server.publish("foo/boogagoo", "oh noes!!!");
        checkEqual(connection.payloads, std::vector<std::string>({"interesting stuff", "interesting stuff"}));

        server.unsubscribe(connection, 3, std::vector<std::string>({"foo/bar/+"}));
        const auto unsuback2 = dynamic_cast<MqttUnsuback*>(connection.lastMsg.get());
        checkNotNull(unsuback2);
        checkEqual(unsuback2->msgId, 3);

        server.publish("foo/bar/baz", "interesting stuff");
        server.publish("foo/boogagoo", "oh noes!!!");
        checkEqual(connection.payloads,
                   std::vector<std::string>({"interesting stuff", "interesting stuff"})); //shouldn't have changed
    }
};
REGISTER_TEST(server, Unsubscribe)


struct UnsubscribeHandle: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x2a, //fixed header
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

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());
        server.subscribe(connection, 42, std::vector<std::string>({"foo/bar/+"}));

        server.publish("foo/bar/baz", "interesting stuff");
        server.publish("foo/boogagoo", "oh noes!!!");
        checkEqual(connection.payloads, std::vector<std::string>({"interesting stuff"}));

        bytes = { 0xa2, 0x0d, //fixed header
                0x00, 0x21, //message ID
                0x00, 0x09, 'f', 'o', 'o', '/', 'b', 'a', 'r', '/', '+',
                };

        auto msg = MqttFactory::create(bytes);
        checkNotNull(msg.get());
        msg->handle(server, connection); //unsubscribe
        const auto unsuback = dynamic_cast<MqttUnsuback*>(connection.lastMsg.get());
        checkNotNull(unsuback);
        checkEqual(unsuback->msgId, 33);

        server.publish("foo/bar/baz", "interesting stuff");
        server.publish("foo/boogagoo", "oh noes!!!");
        checkEqual(connection.payloads,
                   std::vector<std::string>({"interesting stuff"})); //shouldn't have changed
    }
};
REGISTER_TEST(server, UnsubscribeHandle)


struct Ping: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x2a, //fixed header
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

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());

        server.ping(connection);
        const auto pingResp = dynamic_cast<MqttPingResp*>(connection.lastMsg.get());
        checkNotNull(pingResp);
    }
};
REGISTER_TEST(server, Ping)


struct PingWithMessage: public TestCase {
    void test() override {
        MqttServer server;
        std::vector<ubyte> bytes{ 0x10, 0x2a, //fixed header
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

        TestMqttConnection connection(bytes);
        server.newConnection(connection, connection.connect.get());

        const auto msg = MqttFactory::create(std::vector<ubyte>{0xc0, 0x00}); //ping request
        msg->handle(server, connection);
        const auto pingResp = dynamic_cast<MqttPingResp*>(connection.lastMsg.get());
        checkNotNull(pingResp);
    }
};
REGISTER_TEST(server, PingWithMessage)
