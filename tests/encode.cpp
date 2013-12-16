#include "unit_thread.hpp"
#include <cereal_all>
#include "message.hpp"

struct CerealiseFixedHeader: public TestCase {
    virtual void test() override {
        Cerealiser cereal;
        cereal << MqttFixedHeader(MqttType::PUBLISH, true, 2, false, 5);
        const auto expected = std::vector<ubyte>{0x3c, 0x5};
        checkEqual(cereal.getBytes(), expected);
    }
};
REGISTER_TEST(encode, CerealiseFixedHeader)

struct DecerealiseMqttHeader: public TestCase {
    virtual void test() override {
        Decerealiser cereal(std::vector<ubyte>{0x3c, 0x5});
        const auto header = cereal.read<MqttFixedHeader>();
        checkEqual((int)header.type, (int)MqttType::PUBLISH);
        checkEqual(header.dup, true);
        checkEqual(header.qos, 2);
        checkEqual(header.retain, false);
        checkEqual(header.remaining, 5);
    }
};
REGISTER_TEST(encode, DecerealiseMqttHeader)

static std::vector<uint8_t> encode(MqttFixedHeader header) {
    Cerealiser cereal;
    cereal << header;
    return cereal.getBytes();
}

static MqttFixedHeader headerFromBytes(std::vector<uint8_t> bytes) {
    Decerealiser cereal(bytes);
    return cereal.value<MqttFixedHeader>();
}


class EncodeFixedHeader: public TestCase {
    virtual void test() override {
        checkEqual(encode(MqttFixedHeader(MqttType::PUBLISH, true, 2, false, 5)),
                   std::vector<uint8_t>({0x3c, 0x5}));
    }
};
REGISTER_TEST(encode, EncodeFixedHeader)


class DecodeFixedHeader: public TestCase {
    virtual void test() override {
        const auto msg = headerFromBytes(std::vector<uint8_t>{0x3c, 0x5, 0, 0, 0, 0, 0});
        checkEqual((int)msg.type, (int)MqttType::PUBLISH);
        checkEqual(msg.dup, true);
        checkEqual(msg.qos, 2);
        checkEqual(msg.retain, false);
        checkEqual(msg.remaining, 5);
    }
};
REGISTER_TEST(encode, DecodeFixedHeader)


class EncodeBigRemaining: public TestCase {
    virtual void test() override {
        {
            const auto msg = MqttFixedHeader(MqttType::SUBSCRIBE, false, 1, true, 261);
            checkEqual((int)msg.type, (int)MqttType::SUBSCRIBE);
            checkEqual(msg.dup, false);
            checkEqual(msg.qos, 1);
            checkEqual(msg.retain, true);
            checkEqual(msg.remaining, 261);
            checkEqual(encode(msg), std::vector<uint8_t>({0x83, 0x85, 0x02}));
        }
        {
            const auto msg = MqttFixedHeader(MqttType::SUBSCRIBE, false, 1, true, 321);
            checkEqual(encode(msg), std::vector<uint8_t>({0x83, 0xc1, 0x02}));
        }
        {
            const auto msg = MqttFixedHeader(MqttType::SUBSCRIBE, false, 1, true, 2097157);
            checkEqual(encode(msg), std::vector<uint8_t>({0x83, 0x85, 0x80, 0x80, 0x01}));
        }
    }
};
REGISTER_TEST(encode, EncodeBigRemaining)

class DecodeBigRemaining: public TestCase {
    virtual void test() override {
        {
            std::vector<ubyte> bytes{0x12, 0xc1, 0x02};
            bytes.resize(bytes.size() + 321);
            const auto hdr = headerFromBytes(bytes);
            checkEqual(hdr.remaining, 321);
        }
        {
            std::vector<ubyte> bytes{0x12, 0x83, 0x02};
            bytes.resize(bytes.size() + 259);
            const auto hdr = headerFromBytes(bytes);
            checkEqual(hdr.remaining, 259);
        }
        {
            std::vector<ubyte> bytes{0x12, 0x85, 0x80, 0x80, 0x01};
            bytes.resize(bytes.size() + 2097157);
            const auto hdr = headerFromBytes(bytes);
            checkEqual(hdr.remaining, 2097157);
        }
    }
};
REGISTER_TEST(encode, DecodeBigRemaining)

// class ConnectMsg: public TestCase {
//     virtual void test() override {
//         std::vector<ubyte> bytes{0x10, 0x2a, //fixed header
//                           0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', //protocol name
//                           0x03, //protocol version
//                           0xcc, //connection flags 1100111x username, pw, !wr, w(01), w, !c, x
//                           0x00, 0x0a, //keepalive of 10
//                           0x00, 0x03, 'c', 'i', 'd', //client ID
//                           0x00, 0x04, 'w', 'i', 'l', 'l', //will topic
//                           0x00, 0x04, 'w', 'm', 's', 'g', //will msg
//                           0x00, 0x07, 'g', 'l', 'i', 'f', 't', 'e', 'l', //username
//                           0x00, 0x02, 'p', 'w', //password
//                 };
//         const auto msg = MqttFactory.create(bytes);
//         checkNotNull(msg);

//         const auto connect = cast(MqttConnect)msg;
//         checkNotNull(connect);

//         checkEqual(connect.protoName, "MQIsdp");
//         checkEqual(connect.protoVersion, 3);
//         checkTrue(connect.hasUserName);
//         checkTrue(connect.hasPassword);
//         checkFalse(connect.hasWillRetain);
//         checkEqual(connect.willQos, 1);
//         checkTrue(connect.hasWill);
//         checkFalse(connect.hasClear);
//         checkEqual(connect.keepAlive, 10);
//         checkEqual(connect.clientId, "cid");
//         checkEqual(connect.willTopic, "will");
//         checkEqual(connect.willMessage, "wmsg");
//         checkEqual(connect.userName, "gliftel");
//         checkEqual(connect.password, "pw");
//     }
// };
// REGISTER_TEST(encode, ConnectMsg)

class ConnackMsg: public TestCase {
    virtual void test() override {
        Cerealiser cereal;
        cereal << MqttConnack(MqttConnack::Code::SERVER_UNAVAILABLE);
        checkEqual(cereal.getBytes(), std::vector<ubyte>({0x20, 0x2, 0x0, 0x3}));
    }
};
REGISTER_TEST(encode, ConnackMsg)


// class DecodePublishWithMsgId: public TestCase {
//     virtual void test() override {

//         std::vector<ubyte> bytes{0x3c, 0x0b, //fixed header
//                       0x00, 0x03, 't', 'o', 'p', //topic name
//                       0x00, 0x21, //message ID
//                       'b', 'o', 'r', 'g', //payload
//                 };

//     const msg = MqttFactory.create(bytes);
//     checkNotNull(msg);

//     const publish = cast(MqttPublish)msg;
//     checkNotNull(publish);

//     checkEqual(publish.topic, "top");
//     checkEqual(publish.payload, "borg");
//     checkEqual(publish.msgId, 33);
//     }
// };
// REGISTER_TEST(encode, DecodePublishWithMsgId)


// // class DecodePublishWithNoMsgId: public TestCase {
// //     virtual void test() override {
// //         ubyte[] bytes = [ 0x30, 0x05, //fixed header
// //                           0x00, 0x03, 't', 'u', 'p', //topic name
// //             ];
// //         const msg = MqttFactory.create(bytes);
// //         checkNotNull(msg);
// //         const publish = cast(MqttPublish)msg;
// //         checkNotNull(publish);
// //         checkEqual(publish.topic, "tup");
// //         checkEqual(publish.msgId, 0); //no message id
// //     }
// // };
// // REGISTER_TEST(encode, DecodePublishWithNoMsgId)


// // class DecodePublishWithBadSize: public TestCase {
// //     virtual void test() override {
// //         ubyte[] bytes = [ 0x30, 0x60, //fixed header with wrong (too big) size
// //                           0x00, 0x03, 't', 'u', 'p', //topic name
// //                           'b', 'o', 'r', 'g', //payload
// //             ];
// //         const msg = MqttFactory.create(bytes);
// //         checkNull(msg);
// //     }
// // };
// // REGISTER_TEST(encode, DecodePublishWithBadSize)


template<typename T>
static std::vector<ubyte> encodeMsg(T msg) {
    Cerealiser cereal;
    cereal << msg;
    return cereal.getBytes();
}


class EncodePublish: public TestCase {
    virtual void test() override {
        std::vector<ubyte> info{'i', 'n', 'f', 'o'};
        checkEqual(encodeMsg(MqttPublish(false, 2, true, "foo", info, 12)),
                   std::vector<ubyte>({0x35, 0x0b, //header
                    0x00, 0x03, 'f', 'o', 'o', //topic
                    0x00, 0x0c, //msgId
                               'i', 'n', 'f', 'o'}));
        std::vector<ubyte> boo{'b', 'o', 'o'};
        checkEqual(encodeMsg(MqttPublish(true, 0, false, "bars", boo)),
                   std::vector<ubyte>({0x38, 0x09, //header
                    0x00, 0x04, 'b', 'a', 'r', 's',//topic
                               'b', 'o', 'o'}));
        std::vector<ubyte> payload{'p', 'a', 'y', 'l', 'o', 'a', 'd'};
        checkEqual(encodeMsg(MqttPublish("topic", payload)),
                   std::vector<ubyte>({0x30, 0x0e, //header
                    0x00, 0x05, 't', 'o', 'p', 'i', 'c',
                               'p', 'a', 'y', 'l', 'o', 'a', 'd'}));
    }
};
REGISTER_TEST(encode, EncodePublish)


// // class Subscribe: public TestCase {
// //     virtual void test() override {
// //         ubyte[] bytes = [ 0x8c, 0x13, //fixed header
// //                           0x00, 0x21, //message ID
// //                           0x00, 0x05, 'f', 'i', 'r', 's', 't',
// //                           0x01, //qos
// //                           0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd',
// //                           0x02, //qos
// //             ];
// //         const msg = MqttFactory.create(bytes);
// //         checkNotNull(msg);
// //         const subscribe = cast(MqttSubscribe)msg;
// //         checkNotNull(subscribe);
// //         checkEqual(subscribe.msgId, 33);
// //         checkEqual(subscribe.topics,
// //                    [MqttSubscribe.Topic("first", 1), MqttSubscribe.Topic("second", 2)]);
// //     }
// // };
// // REGISTER_TEST(encode, Subscribe)


class Suback: public TestCase {
    virtual void test() override {
        checkEqual(encodeMsg(MqttSuback(12, std::vector<ubyte>({1, 2, 0, 2}))),
                   std::vector<ubyte>({0x90, 0x06, //fixed header
                    0x00, 0x0c, //msgId
                               0x01, 0x02, 0x00, 0x02})); //qos
    }
};
REGISTER_TEST(encode, Suback)


// class PingReq: public TestCase {
//     virtual void test() override {
//         ubyte[] bytes = [ 0xc0, 0x00 ];
//         const pingReq = MqttFactory.create(bytes);
//         checkNotNull(pingReq);
//     }
// };
// REGISTER_TEST(encode, PingReq)


// class PingResp: public TestCase {
//     virtual void test() override {
//         checkEqual((new MqttPingResp()).encode(),
//                    [0xd0, 0x00]);
//     }
// };
// REGISTER_TEST(encode, PingResp)


// class Unsubscribe: public TestCase {
//     virtual void test() override {
//         ubyte[] bytes = [ 0xa2, 0x11, //fixed header
//                           0x00, 0x2a, //message ID
//                           0x00, 0x05, 'f', 'i', 'r', 's', 't',
//                           0x00, 0x06, 's', 'e', 'c', 'o', 'n', 'd',
//             ];
//         const msg = MqttFactory.create(bytes);
//         checkNotNull(msg);
//         const unsubscribe = cast(MqttUnsubscribe)msg;
//         checkNotNull(unsubscribe);
//         checkEqual(unsubscribe.msgId, 42);
//         checkEqual(unsubscribe.topics, ["first", "second"]);
//     }
// };
// REGISTER_TEST(encode, Unsubscribe)


// class Unsuback: public TestCase {
//     virtual void test() override {
//         checkEqual((new MqttUnsuback(13)).encodeMsg(),
//                    [0xb0, 0x02, //fixed header
//                     0x00, 0x0d ]); //msgId
//     }
// };
// REGISTER_TEST(encode, Unsuback)
