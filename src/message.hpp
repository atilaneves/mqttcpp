#ifndef MESSAGE_H_
#define MESSAGE_H_

class Cereal;
class MqttServer;
class MqttConnection;

#include "dtypes.hpp"
#include <vector>
#include <string>

enum class MqttType {
    RESERVED1   = 0,
    CONNECT     = 1,
    CONNACK     = 2,
    PUBLISH     = 3,
    PUBACK      = 4,
    PUBREC      = 5,
    PUBREL      = 6,
    PUBCOMP     = 7,
    SUBSCRIBE   = 8,
    SUBACK      = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK    = 11,
    PINGREQ     = 12,
    PINGRESP    = 13,
    DISCONNECT  = 14,
    RESERVED2   = 15
};

class MqttFixedHeader {
public:
    enum {SIZE = 2};

    MqttType type;
    bool dup;
    ubyte qos;
    bool retain;
    uint remaining;

    MqttFixedHeader();
    MqttFixedHeader(MqttType t, bool d, ubyte q, bool rt, uint re);

    void cerealise(Cereal& cereal);

private:

    uint getRemainingSize(Cereal& cereal);
    void setRemainingSize(Cereal& cereal) const;
};


class MqttMessage {
public:
    virtual void handle(MqttServer& server, MqttConnection& connection) const;
};


class MqttConnect: public MqttMessage {
 public:

    MqttConnect(MqttFixedHeader h);

    void cerealise(Cereal& cereal);
    bool isBadClientId() const { return clientId.length() < 1 || clientId.length() > 23; }

    MqttFixedHeader header;
    std::string protoName;
    ubyte protoVersion;
    bool hasUserName; //1
    bool hasPassword; //1
    bool hasWillRetain; //1
    ubyte willQos; //2
    bool hasWill; //1
    bool hasClear; //1
    bool reserved; //1
    ushort keepAlive;
    std::string clientId;
    std::string willTopic;
    std::string willMessage;
    std::string userName;
    std::string password;
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

    MqttConnack();
    MqttConnack(Code c);

    void cerealise(Cereal& cereal);

    MqttFixedHeader header;
    ubyte reserved;
    Code code;
};


class MqttSubscribe: public MqttMessage {
public:

    MqttSubscribe(MqttFixedHeader h);

    void handle(MqttServer& server, MqttConnection& connection) const override;

    struct Topic {
        std::string topic;
        ubyte qos;
        void cerealise(Cereal& cereal);
    };

    void cerealise(Cereal& cereal);

    MqttFixedHeader header;
    ushort msgId;
    std::vector<Topic> topics;
};

class MqttSuback: public MqttMessage {
public:

    MqttSuback(MqttFixedHeader h);
    MqttSuback(ushort m, std::vector<ubyte> q);

    void cerealise(Cereal& cereal);

    MqttFixedHeader header;
    ushort msgId;
    std::vector<ubyte> qos;
};


class MqttUnsubscribe: public MqttMessage {
public:
    MqttUnsubscribe(MqttFixedHeader h);

    virtual void handle(MqttServer& server, MqttConnection& connection) const override;
    void cerealise(Cereal& cereal);

    MqttFixedHeader header;
    ushort msgId;
    std::vector<std::string> topics;
};


class MqttUnsuback: public MqttMessage {
public:

    MqttUnsuback(ushort m);
    MqttUnsuback(MqttFixedHeader h);

    void cerealise(Cereal& cereal);

    MqttFixedHeader header;
    ushort msgId;
};


class MqttPublish: public MqttMessage {
public:

    MqttPublish(MqttFixedHeader h);
    MqttPublish(std::string topic, std::vector<ubyte> payload, ushort msgId = 0);
    MqttPublish(bool dup, ubyte qos, bool retain, std::string t, std::vector<ubyte> p, ushort mid = 0);

    void cerealise(Cereal& cereal);
    void handle(MqttServer& server, MqttConnection& connection) const override;

    MqttFixedHeader header;
    std::string topic;
    std::vector<ubyte> payload;
    ushort msgId;
};

class MqttDisconnect: public MqttMessage {
public:
    void handle(MqttServer& server, MqttConnection& connection) const override;
};

class MqttPingReq: public MqttMessage {
public:
    void handle(MqttServer& server, MqttConnection& connection) const override;
};

class MqttPingResp: public MqttMessage {
public:
    std::vector<ubyte> encode() const;
};


#endif // MESSAGE_H_
