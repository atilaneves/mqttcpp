#include "server.hpp"


void MqttServer::publish(std::string topic, std::vector<ubyte> payload) {
    (void)topic;
    (void)payload;
}

void MqttServer::subscribe(MqttConnection& connection, ushort msgId, std::vector<MqttSubscribe::Topic> topics) {
    (void)connection;
    (void)msgId;
    (void)topics;
}
