#include "server.hpp"
#include "Cerealiser.hpp"


template<typename T>
std::vector<ubyte> encode(T msg) {
    Cerealiser cereal;
    cereal << msg;
    return cereal.getBytes();
}


void MqttServer::publish(std::string topic, std::vector<ubyte> payload) {
    (void)topic;
    (void)payload;
}

void MqttServer::subscribe(MqttConnection& connection, ushort msgId,
                           std::vector<MqttSubscribe::Topic> topics) {
    (void)connection;
    (void)msgId;
    (void)topics;
}

void MqttServer::unsubscribe(MqttConnection& connection, ushort msgId,
                             std::vector<std::string> topics) {
    (void)connection;
    (void)msgId;
    (void)topics;
}

void MqttServer::ping(MqttConnection& connection) {
    (void)connection;
}

void MqttServer::unsubscribe(MqttConnection& connection) {
    (void)connection;
}


void MqttConnection::newMessage(std::string topic, std::vector<ubyte> payload) {
    write(encode(MqttPublish(topic, payload)));
}

void MqttConnection::read(std::vector<ubyte> bytes) {
    (void)bytes;
}
