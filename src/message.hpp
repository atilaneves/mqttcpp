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
     virtual void handle(MqttServer& server, MqttConnection& connection) const {
         (void)server;
         (void)connection;
     }
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



#endif // MESSAGE_H_
