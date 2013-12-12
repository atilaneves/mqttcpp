#ifndef MESSAGE_H_
#define MESSAGE_H_

class Cereal;
class MqttServer;
class MqttConnection;

#include "dtypes.hpp"

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


class MqttMessage {
public:
     virtual void handle(MqttServer& server, MqttConnection& connection) const {
         (void)server;
         (void)connection;
     }
};



#endif // MESSAGE_H_
