#ifndef SERVER_H_
#define SERVER_H_

#include "dtypes.hpp"
#include "message.hpp"
#include <string>
#include <vector>


class MqttConnection;

class MqttServer {
public:

    void publish(std::string topic, std::vector<ubyte> payload);
    void subscribe(MqttConnection& connection, ushort msgId,
                   std::vector<MqttSubscribe::Topic> topics);
    void unsubscribe(MqttConnection& connection, ushort msgId,
                     std::vector<std::string> topics);
};



#endif // SERVER_H_
