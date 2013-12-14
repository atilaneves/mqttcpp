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

// // private auto encode(in MqttFixedHeader header) {
// //     Cerealiser cereal;
// //     cereal << cast(MqttFixedHeader) header;
// //     return cereal.getBytes();
// // }

// // private auto headerFromBytes(in ubyte[] bytes) {
// //     Decerealiser cereal(bytes);
// //     return cereal.value!MqttFixedHeader;
// // }


// // class EncodeFixedHeader: public TestCase {
// //     virtual void test() override {
// //         const msg = MqttFixedHeader(MqttType::PUBLISH, true, 2, false, 5);
// //         checkEqual(msg.encode(), [0x3c, 0x5]);
// //     }
// // };
// // REGISTER_TEST(encode, EncodeFixedHeader)


// // class DecodeFixedHeader: public TestCase {
// //     virtual void test() override {
// //         const msg = headerFromBytes([0x3c, 0x5, 0, 0, 0, 0, 0]);
// //         checkEqual(msg.type, MqttType::PUBLISH);
// //         checkEqual(msg.dup, true);
// //         checkEqual(msg.qos, 2);
// //         checkEqual(msg.retain, false);
// //         checkEqual(msg.remaining, 5);
// //     }
// // };
// // REGISTER_TEST(encode, DecodeFixedHeader)

// // class EncodeBigRemaining: public TestCase {
// //     virtual void test() override {
// //         {
// //             const msg = MqttFixedHeader(MqttType::SUBSCRIBE, false, 1, true, 261);
// //             checkEqual(msg.type, MqttType::SUBSCRIBE);
// //             checkEqual(msg.dup, false);
// //             checkEqual(msg.qos, 1);
// //             checkEqual(msg.retain, true);
// //             checkEqual(msg.remaining, 261);
// //             checkEqual(msg.encode(), [0x83, 0x85, 0x02]);
// //         }
// //         {
// //             const msg = MqttFixedHeader(MqttType::SUBSCRIBE, false, 1, true, 321);
// //             checkEqual(msg.encode(), [0x83, 0xc1, 0x02]);
// //         }
// //         {
// //             const msg = MqttFixedHeader(MqttType::SUBSCRIBE, false, 1, true, 2_097_157);
// //             checkEqual(msg.encode(), [0x83, 0x85, 0x80, 0x80, 0x01]);
// //         }
// //     }
// // };
// // REGISTER_TEST(encode, EncodeBigRemaining)

// // class DecodeBigRemaining: public TestCase {
// //     virtual void test() override {
// //         {
// //             ubyte[] bytes = [0x12, 0xc1, 0x02];
// //             bytes.length += 321;
// //             const hdr = headerFromBytes(bytes);
// //             checkEqual(hdr.remaining, 321);
// //         }
// //         {
// //             ubyte[] bytes = [0x12, 0x83, 0x02];
// //             bytes.length += 259;
// //             const hdr = headerFromBytes(bytes);
// //             checkEqual(hdr.remaining, 259);
// //         }
// //         {
// //             ubyte[] bytes = [0x12, 0x85, 0x80, 0x80, 0x01];
// //             bytes.length += 2_097_157;
// //             const hdr = headerFromBytes(bytes);
// //             checkEqual(hdr.remaining, 2_097_157);
// //         }
// //     }
// // };
// // REGISTER_TEST(encode, DecodeBigRemaining)

// // class ConnectMsg: public TestCase {
// //     virtual void test() override {
// //         ubyte[] bytes = [ 0x10, 0x2a, //fixed header
// //                           0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', //protocol name
// //                           0x03, //protocol version
// //                           0xcc, //connection flags 1100111x username, pw, !wr, w(01), w, !c, x
// //                           0x00, 0x0a, //keepalive of 10
// //                           0x00, 0x03, 'c', 'i', 'd', //client ID
// //                           0x00, 0x04, 'w', 'i', 'l', 'l', //will topic
// //                           0x00, 0x04, 'w', 'm', 's', 'g', //will msg
// //                           0x00, 0x07, 'g', 'l', 'i', 'f', 't', 'e', 'l', //username
// //                           0x00, 0x02, 'p', 'w', //password
// //             ];
// //         const msg = MqttFactory.create(bytes);
// //         checkNotNull(msg);

// //         const connect = cast(MqttConnect)msg;
// //         checkNotNull(connect);

// //         checkEqual(connect.protoName, "MQIsdp");
// //         checkEqual(connect.protoVersion, 3);
// //         checkTrue(connect.hasUserName);
// //         checkTrue(connect.hasPassword);
// //         checkFalse(connect.hasWillRetain);
// //         checkEqual(connect.willQos, 1);
// //         checkTrue(connect.hasWill);
// //         checkFalse(connect.hasClear);
// //         checkEqual(connect.keepAlive, 10);
// //         checkEqual(connect.clientId, "cid");
// //         checkEqual(connect.willTopic, "will");
// //         checkEqual(connect.willMessage, "wmsg");
// //         checkEqual(connect.userName, "gliftel");
// //         checkEqual(connect.password, "pw");
// //     }
// // };
// // REGISTER_TEST(encode, ConnectMsg)

// // class ConnackMsg: public TestCase {
// //     virtual void test() override {
// //         Cerealiser cereal;
// //         cereal << new MqttConnack(MqttConnack.Code.SERVER_UNAVAILABLE);
// //         checkEqual(cereal.getBytes(), [0x20, 0x2, 0x0, 0x3]);
// //     }
// // };
// // REGISTER_TEST(encode, ConnackMsg)


// // class DecodePublishWithMsgId: public TestCase {
// //     virtual void test() override {

// //     ubyte[] bytes = [ 0x3c, 0x0b, //fixed header
// //                       0x00, 0x03, 't', 'o', 'p', //topic name
// //                       0x00, 0x21, //message ID
// //                       'b', 'o', 'r', 'g', //payload
// //         ];

// //     const msg = MqttFactory.create(bytes);
// //     checkNotNull(msg);

// //     const publish = cast(MqttPublish)msg;
// //     checkNotNull(publish);

// //     checkEqual(publish.topic, "top");
// //     checkEqual(publish.payload, "borg");
// //     checkEqual(publish.msgId, 33);
// //     }
// // };
// // REGISTER_TEST(encode, DecodePublishWithMsgId)


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


// // private auto encodeMsg(T)(T msg) {
// //     Cerealiser cereal;
// //     cereal << msg;
// //     return cereal.getBytes();
// // }



// // class EncodePublish: public TestCase {
// //     virtual void test() override {
// //         checkEqual((new MqttPublish(false, 2, true, "foo", cast(ubyte[])"info", 12)).encodeMsg(),
// //                    [0x35, 0x0b, //header
// //                     0x00, 0x03, 'f', 'o', 'o', //topic
// //                     0x00, 0x0c, //msgId
// //                     'i', 'n', 'f', 'o',
// //                        ]
// //             );
// //         checkEqual((new MqttPublish(true, 0, false, "bars", cast(ubyte[])"boo")).encodeMsg(),
// //                    [0x38, 0x09, //header
// //                     0x00, 0x04, 'b', 'a', 'r', 's',//topic
// //                     'b', 'o', 'o',
// //                        ]
// //             );
// //         checkEqual((new MqttPublish("topic", cast(ubyte[])"payload")).encodeMsg(),
// //                    [0x30, 0x0e, //header
// //                     0x00, 0x05, 't', 'o', 'p', 'i', 'c',
// //                     'p', 'a', 'y', 'l', 'o', 'a', 'd']);
// //     }
// // };
// // REGISTER_TEST(encode, EncodePublish)


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


// class Suback: public TestCase {
//     virtual void test() override {
//         checkEqual((new MqttSuback(12, [1, 2, 0, 2])).encodeMsg(),
//                    [0x90, 0x06, //fixed header
//                     0x00, 0x0c, //msgId
//                     0x01, 0x02, 0x00, 0x02, //qos
//                        ]);
//     }
// };
// REGISTER_TEST(encode, Suback)


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
