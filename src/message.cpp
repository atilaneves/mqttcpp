#include "message.hpp"
#include "Cereal.hpp"
#include <vector>

using namespace std;


class MqttFixedHeader {
public:
    enum {SIZE = 2};

    MqttType type;
    bool dup;
    ubyte qos;
    bool retain;
    uint remaining;

    void cerealise(Cereal& cereal) {
        //custom serialisation needed due to remaining size field
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
        //TODO: do bits
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

// class MqttConnack: MqttMessage {

//     enum Code: byte {
//         ACCEPTED = 0,
//         BAD_VERSION = 1,
//         BAD_ID = 2,
//         SERVER_UNAVAILABLE = 3,
//         BAD_USER_OR_PWD = 4,
//         NO_AUTH = 5,
//     }

//     this() {
//         header = MqttFixedHeader(MqttType.CONNACK, false, 0, false, 2);
//     }

//     this(Code code) {
//         this.code = code;
//         this();
//     }

//     MqttFixedHeader header;
//     ubyte reserved;
//     Code code;
// };


// class MqttPublish: MqttMessage {
// public:
//     this(MqttFixedHeader header) {
//         this.header = header;
//     }

//     this(in string topic, in ubyte[] payload, ushort msgId = 0) {
//         this(false, 0, false, topic, payload, msgId);
//     }

//     this(in bool dup, in ubyte qos, in bool retain, in string topic, in ubyte[] payload, in ushort msgId = 0) {
//         const topicLen = cast(uint)topic.length + 2; //2 for length
//         auto remaining = qos ? topicLen + 2 /*msgId*/ : topicLen;
//         remaining += payload.length;

//         this.header = MqttFixedHeader(MqttType.PUBLISH, dup, qos, retain, remaining);
//         this.topic = topic;
//         this.payload = payload.dup;
//         this.msgId = msgId;
//     }

//     void postBlit(Cereal cereal) {
//         auto payloadLen = header.remaining - (topic.length + MqttFixedHeader.SIZE);
//         if(header.qos) {
//             if(header.remaining < 7 && cereal.type == Cereal.Type.Read) {
//                 stderr.writeln("Error: PUBLISH message with QOS but no message ID");
//             } else {
//                 cereal.grain(msgId);
//                 payloadLen -= 2;
//             }
//         }
//         if(cereal.type == Cereal.Type.Read) payload.length = payloadLen;
//         foreach(ref b; payload) cereal.grain(b);
//     }

//     override void handle(MqttServer server, MqttConnection connection) const {
//         server.publish(topic, payload);
//     }

//     MqttFixedHeader header;
//     string topic;
//     @NoCereal ubyte[] payload;
//     @NoCereal ushort msgId;
// }


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

//     this(in ushort msgId, in ubyte[] qos) {
//         this.header = MqttFixedHeader(MqttType.SUBACK, false, 0, false, cast(uint)qos.length + 2);
//         this.msgId = msgId;
//         this.qos = qos.dup;
//     }

//     MqttFixedHeader header;
//     ushort msgId;
//     @RawArray ubyte[] qos;
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
//     const(ubyte[]) encode() const {
//         return [0xd0, 0x00];
//     }
// }
