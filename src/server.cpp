#include "server.hpp"
#include "Cerealiser.hpp"
#include <iostream>
#include <algorithm>

template<typename T>
std::vector<ubyte> encode(T msg) {
    Cerealiser cereal;
    cereal << msg;
    return cereal.getBytes();
}


void MqttConnection::newMessage(std::string topic, const std::vector<ubyte>& payload) {
    write(encode(MqttPublish(topic, payload)));
}



void MqttServer::newConnection(MqttConnection& connection,
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

void MqttServer::subscribe(MqttConnection& connection, ushort msgId,
                           std::vector<std::string> topics) {
    const auto qos = 0;
    std::vector<MqttSubscribe::Topic> realTopics;
    std::transform(topics.cbegin(), topics.cend(), std::back_inserter(realTopics),
                   [](std::string t) { return MqttSubscribe::Topic(t, qos); });
    subscribe(connection, msgId, realTopics);
}


void MqttServer::subscribe(MqttConnection& connection, ushort msgId,
                           std::vector<MqttSubscribe::Topic> topics) {
    std::vector<ubyte> qos;
    std::transform(topics.begin(), topics.end(), std::back_inserter(qos),
                   [](MqttSubscribe::Topic t) { return t.qos; });
    connection.write(encode(MqttSuback(msgId, qos)));
    _broker.subscribe(connection, topics);
}


void MqttServer::unsubscribe(MqttConnection& connection) {
    _broker.unsubscribe(connection);
}


void MqttServer::unsubscribe(MqttConnection& connection, ushort msgId,
                             std::vector<std::string> topics) {
    connection.write(encode(MqttUnsuback(msgId)));
    _broker.unsubscribe(connection, topics);
}

void MqttServer::publish(std::string topic, std::string payload) {
    _broker.publish(topic, payload);
}


void MqttServer::publish(std::string topic, std::vector<ubyte> payload) {
    _broker.publish(topic, payload);
}

void MqttServer::ping(MqttConnection& connection) {
    static MqttPingResp msg;
    connection.write(msg.encode());
}
