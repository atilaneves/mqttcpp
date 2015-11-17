#include "server.hpp"
#include "Cerealiser.hpp"
#include <iostream>
#include <algorithm>

template<typename T>
std::vector<ubyte> encode(const T& msg) {
    Cerealiser cereal;
    cereal << msg;
    return cereal.getBytes();
}

void OldMqttConnection::newMessage(const std::string& topic, const std::vector<ubyte>& payload) {
    Cerealiser cereal;
    cereal << MqttPublish(topic, payload);
    write(cereal.getBytes());
}



void OldMqttServer::newConnection(OldMqttConnection& connection,
                               const MqttConnect* connect) {
    if(!connect) {
        std::cerr << "Invalid connect message" << std::endl;
        return;
    }
    auto code = MqttConnack::Code::ACCEPTED;
    if(connect->isBadClientId()) {
        code = MqttConnack::Code::BAD_ID;
    }

    connection.write(encode(MqttConnack(code)));
}

void OldMqttServer::subscribe(OldMqttConnection& connection, ushort msgId,
                           std::vector<std::string> topics) {
    const auto qos = 0;
    std::vector<MqttSubscribe::Topic> realTopics;
    std::transform(topics.cbegin(), topics.cend(), std::back_inserter(realTopics),
                   [](std::string t) { return MqttSubscribe::Topic(t, qos); });
    subscribe(connection, msgId, realTopics);
}


void OldMqttServer::subscribe(OldMqttConnection& connection, ushort msgId,
                           std::vector<MqttSubscribe::Topic> topics) {
    std::vector<ubyte> qos;
    std::transform(topics.begin(), topics.end(), std::back_inserter(qos),
                   [](MqttSubscribe::Topic t) { return t.qos; });
    connection.write(encode(MqttSuback(msgId, qos)));
    _broker.subscribe(connection, topics);
}


void OldMqttServer::unsubscribe(OldMqttConnection& connection) {
    _broker.unsubscribe(connection);
}


void OldMqttServer::unsubscribe(OldMqttConnection& connection, ushort msgId,
                             std::vector<std::string> topics) {
    connection.write(encode(MqttUnsuback(msgId)));
    _broker.unsubscribe(connection, topics);
}

void OldMqttServer::publish(const std::string& topic, const std::string& payload) {
    _broker.publish(topic, payload);
}


void OldMqttServer::publish(const std::string& topic, const std::vector<ubyte>& payload) {
    _broker.publish(topic, payload);
}

void OldMqttServer::ping(OldMqttConnection& connection) {
    static MqttPingResp msg;
    connection.write(msg.encode());
}
