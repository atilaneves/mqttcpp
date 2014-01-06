#ifndef SERVER_H_
#define SERVER_H_

#include "dtypes.hpp"
#include "message.hpp"
#include "broker.hpp"
#include <string>
#include <vector>


class MqttConnection: public MqttSubscriber {
public:
    virtual void newMessage(std::string topic, std::vector<ubyte> payload) override;
    virtual void write(std::vector<ubyte> bytes) = 0;
    virtual void disconnect() = 0;
};


class MqttServer {
public:

    void newConnection(MqttConnection& connection,
                       MqttConnect* connect);
    void subscribe(MqttConnection& connection, ushort msgId,
                   std::vector<std::string> topics);
    void subscribe(MqttConnection& connection, ushort msgId,
                   std::vector<MqttSubscribe::Topic> topics);
    void unsubscribe(MqttConnection& connection);
    void unsubscribe(MqttConnection& connection, ushort msgId,
                     std::vector<std::string> topics);
    void publish(std::string topic, std::string payload);
    void publish(std::string topic, std::vector<ubyte> payload);
    void ping(MqttConnection& connection);
    void useCache(bool u) { _broker.useCache(u); }

private:
    MqttBroker _broker;
};


#endif // SERVER_H_
