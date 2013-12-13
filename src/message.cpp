#include "message.hpp"
#include "server.hpp"
#include "Cereal.hpp"
#include <vector>
#include <iostream>

using namespace std;


class MqttFixedHeader {
public:
    enum {SIZE = 2};

    MqttType type;
    bool dup;
    ubyte qos;
    bool retain;
    uint remaining;

    MqttFixedHeader():
        type(), dup(), qos(), retain(), remaining() {
    }

    MqttFixedHeader(MqttType t, bool d, ubyte q, bool rt, uint re):
        type(t), dup(d), qos(q), retain(rt), remaining(re) {
    }

    void cerealise(Cereal& cereal) {
        cereal.grainBits(type, 4);
        cereal.grainBits(dup, 1);
        cereal.grainBits(qos, 2);
        cereal.grainBits(retain, 1);

        switch(cereal.getType()) {
        case Cereal::Type::Write:
            setRemainingSize(cereal);
            break;

        case Cereal::Type::Read:
            remaining = getRemainingSize(cereal);
            break;
        }
    }

private:

    uint getRemainingSize(Cereal& cereal) {
        //algorithm straight from the MQTT spec
        int multiplier = 1;
        uint value = 0;
        ubyte digit;
        do {
            cereal.grain(digit);
            value += (digit & 127) * multiplier;
            multiplier *= 128;
        } while((digit & 128) != 0);

        return value;
    }

    void setRemainingSize(Cereal& cereal) const {
        //algorithm straight from the MQTT spec
        vector<ubyte> digits;
        uint x = remaining;
        do {
            ubyte digit = x % 128;
            x /= 128;
            if(x > 0) {
                digit = digit | 0x80;
            }
            digits.push_back(digit);
        } while(x > 0);

        for(auto b: digits) cereal.grain(b);
    }
};

class MqttConnect: public MqttMessage {
 public:

    MqttConnect(MqttFixedHeader h):header(h) { }

    void cerealise(Cereal& cereal) {
        cereal.grain(header);
        cereal.grain(protoName);
        cereal.grain(protoVersion);

        cereal.grainBits(hasUserName, 1);
        cereal.grainBits(hasPassword, 1);
        cereal.grainBits(hasWillRetain, 1);
        cereal.grainBits(willQos, 2);
        cereal.grainBits(hasWill, 1);
        cereal.grainBits(hasClear, 1);
        cereal.grainBits(reserved, 1);

        cereal.grain(keepAlive);
        cereal.grain(clientId);

        if(hasWill) cereal.grain(willTopic);
        if(hasWill) cereal.grain(willMessage);
        if(hasUserName) cereal.grain(userName);
        if(hasPassword) cereal.grain(password);
    }

    bool isBadClientId() const { return clientId.length() < 1 || clientId.length() > 23; }

    MqttFixedHeader header;
    string protoName;
    ubyte protoVersion;
    bool hasUserName; //1
    bool hasPassword; //1
    bool hasWillRetain; //1
    ubyte willQos; //2
    bool hasWill; //1
    bool hasClear; //1
    bool reserved; //1
    ushort keepAlive;
    string clientId;
    string willTopic;
    string willMessage;
    string userName;
    string password;
};


class MqttConnack: public MqttMessage {
public:
    enum class Code {
        ACCEPTED = 0,
        BAD_VERSION = 1,
        BAD_ID = 2,
        SERVER_UNAVAILABLE = 3,
        BAD_USER_OR_PWD = 4,
        NO_AUTH = 5,
    };

    MqttConnack():
        header(MqttType::CONNACK, false, 0, false, 2) {
    }

    MqttConnack(Code c):
        MqttConnack() {
        code = c;
    }

    void cerealise(Cereal& cereal) {
        cereal.grain(header);
        cereal.grain(reserved);
        cereal.grainBits(code, 8);
    }

    MqttFixedHeader header;
    ubyte reserved;
    Code code;
};


class MqttPublish: public MqttMessage {
public:
    MqttPublish(MqttFixedHeader h):header(h) {

    }

    MqttPublish(string topic, std::vector<ubyte> payload, ushort msgId = 0):
        MqttPublish(false, 0, false, topic, payload, msgId) {
    }

    MqttPublish(bool dup, ubyte qos, bool retain, string t, std::vector<ubyte> p, ushort mid = 0) {
        const auto topicLen = t.length() + 2; //2 for length
        auto remaining = qos ? topicLen + 2 /*msgId*/ : topicLen;
        remaining += p.size();

        header = MqttFixedHeader(MqttType::PUBLISH, dup, qos, retain, remaining);
        topic = t;
        payload = std::move(p);
        msgId = mid;
    }

    void cerealise(Cereal& cereal) {
        cereal.grain(header);
        cereal.grain(topic);

        auto payloadLen = header.remaining - (topic.length() + MqttFixedHeader::SIZE);
        if(header.qos) {
            if(header.remaining < 7 && cereal.getType() == Cereal::Type::Read) {
                cerr << "Error: PUBLISH message with QOS but no message ID" << endl;
            } else {
                cereal.grain(msgId);
                payloadLen -= 2;
            }
        }
        if(cereal.getType() == Cereal::Type::Read) payload.resize(payloadLen);
        for(auto& b: payload) cereal.grain(b);
    }

    void handle(MqttServer& server, MqttConnection& connection) const override {
        (void)connection;
        server.publish(topic, payload);
    }

    MqttFixedHeader header;
    string topic;
    std::vector<ubyte> payload;
    ushort msgId;
};


// class MqttSubscribe: MqttMessage {
// public:
//     this(MqttFixedHeader header) {
//         this.header = header;
//     }

//     override void handle(MqttServer server, MqttConnection connection) const {
//         server.subscribe(connection, msgId, topics);
//     }

//     static struct Topic {
//         string topic;
//         ubyte qos;
//     }

//     MqttFixedHeader header;
//     ushort msgId;
//     @RawArray Topic[] topics;
// }

// class MqttSuback: MqttMessage {
// public:

//     this(MqttFixedHeader header) {
//         this.header = header;
//     }

//     this(in ushort msgId, in std::vector<ubyte> qos) {
//         this.header = MqttFixedHeader(MqttType.SUBACK, false, 0, false, cast(uint)qos.length + 2);
//         this.msgId = msgId;
//         this.qos = qos.dup;
//     }

//     MqttFixedHeader header;
//     ushort msgId;
//     @RawArray std::vector<ubyte> qos;
// }

// class MqttUnsubscribe: MqttMessage {
//     this(MqttFixedHeader header) {
//         this.header = header;
//     }

//     override void handle(MqttServer server, MqttConnection connection) const {
//         server.unsubscribe(connection, msgId, topics);
//     }

//     MqttFixedHeader header;
//     ushort msgId;
//     @RawArray string[] topics;
// }

// class MqttUnsuback: MqttMessage {
//     this(in ushort msgId) {
//         this.header = MqttFixedHeader(MqttType.UNSUBACK, false, 0, false, 2);
//         this.msgId = msgId;
//     }

//     this(MqttFixedHeader header) {
//         this.header = header;
//     }

//     MqttFixedHeader header;
//     ushort msgId;
// }

// class MqttDisconnect: MqttMessage {
//     override void handle(MqttServer server, MqttConnection connection) const {
//         server.unsubscribe(connection);
//         connection.disconnect();
//     }
// }

// class MqttPingReq: MqttMessage {
//     override void handle(MqttServer server, MqttConnection connection) const {
//         server.ping(connection);
//     }
// }

// class MqttPingResp: MqttMessage {
//     const(std::vector<ubyte>) encode() const {
//         return [0xd0, 0x00];
//     }
// }
