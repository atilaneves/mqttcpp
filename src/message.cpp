#include "message.hpp"
#include "server.hpp"
#include "Cereal.hpp"
#include <vector>
#include <iostream>

using namespace std;


MqttFixedHeader::MqttFixedHeader():
    type(), dup(), qos(), retain(), remaining() {
}

MqttFixedHeader::MqttFixedHeader(MqttType t, bool d, ubyte q, bool rt, uint re):
    type(t), dup(d), qos(q), retain(rt), remaining(re) {
}

void MqttFixedHeader::cerealise(Cereal& cereal) {
    cereal.grainBits(type, 4);
    cereal.grainBits(dup, 1);
    cereal.grainBits(qos, 2);
    cereal.grainBits(retain, 1);

    switch(cereal.getType()) {
    case Cereal::Type::Write:
        setRemainingSize(cereal);
        break;

    case Cereal::Type::Read:
        remaining = getRemainingSize(cereal);
        break;
    }
}

uint MqttFixedHeader::getRemainingSize(Cereal& cereal) {
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

void MqttFixedHeader::setRemainingSize(Cereal& cereal) const {
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

void MqttMessage::handle(MqttServer& server, MqttConnection& connection) const {
    (void)server;
    (void)connection;
}


MqttConnect::MqttConnect(MqttFixedHeader h):header(h) { }

void MqttConnect::cerealise(Cereal& cereal) {
    cereal.grain(header);
    cereal.grain(protoName);
    cereal.grain(protoVersion);

    cereal.grainBits(hasUserName, 1);
    cereal.grainBits(hasPassword, 1);
    cereal.grainBits(hasWillRetain, 1);
    cereal.grainBits(willQos, 2);
    cereal.grainBits(hasWill, 1);
    cereal.grainBits(hasClear, 1);
    cereal.grainBits(reserved, 1);

    cereal.grain(keepAlive);
    cereal.grain(clientId);

    if(hasWill) cereal.grain(willTopic);
    if(hasWill) cereal.grain(willMessage);
    if(hasUserName) cereal.grain(userName);
    if(hasPassword) cereal.grain(password);
}


MqttConnack::MqttConnack():
    header(MqttType::CONNACK, false, 0, false, 2) {
}

MqttConnack::MqttConnack(Code c):
    MqttConnack() {
    code = c;
}

void MqttConnack::cerealise(Cereal& cereal) {
    cereal.grain(header);
    cereal.grain(reserved);
    cereal.grainBits(code, 8);
}


MqttPublish::MqttPublish(MqttFixedHeader h):header(h) {

}

MqttPublish::MqttPublish(string topic, std::vector<ubyte> payload, ushort msgId):
    MqttPublish(false, 0, false, topic, payload, msgId) {
}

MqttPublish::MqttPublish(bool dup, ubyte qos, bool retain, string t, std::vector<ubyte> p, ushort mid) {
    const auto topicLen = t.length() + 2; //2 for length
    auto remaining = qos ? topicLen + 2 /*msgId*/ : topicLen;
    remaining += p.size();

    header = MqttFixedHeader(MqttType::PUBLISH, dup, qos, retain, remaining);
    topic = t;
    payload = std::move(p);
    msgId = mid;
}

void MqttPublish::cerealise(Cereal& cereal) {
    cereal.grain(header);
    cereal.grain(topic);

    auto payloadLen = header.remaining - (topic.length() + MqttFixedHeader::SIZE);
    if(header.qos) {
        if(header.remaining < 7 && cereal.getType() == Cereal::Type::Read) {
            cerr << "Error: PUBLISH message with QOS but no message ID" << endl;
        } else {
            cereal.grain(msgId);
            payloadLen -= 2;
        }
    }
    if(cereal.getType() == Cereal::Type::Read) payload.resize(payloadLen);
    for(auto& b: payload) cereal.grain(b);
}

void MqttPublish::handle(MqttServer& server, MqttConnection& connection) const {
    (void)connection;
    server.publish(topic, payload);
}


MqttSubscribe::MqttSubscribe(MqttFixedHeader h):header(h) {

}

void MqttSubscribe::handle(MqttServer& server, MqttConnection& connection) const {
    server.subscribe(connection, msgId, topics);
}

void MqttSubscribe::Topic::cerealise(Cereal& cereal) {
    cereal.grain(topic);
    cereal.grain(qos);
}


void MqttSubscribe::cerealise(Cereal& cereal) {
    cereal.grain(header);
    cereal.grain(msgId);
    ushort size;
    cereal.grain(size);
    if(topics.size() != size) topics.resize(size);
    for(auto& t: topics) cereal.grain(t);
}


MqttSuback::MqttSuback(MqttFixedHeader h):header(h) {

}

MqttSuback::MqttSuback(ushort m, std::vector<ubyte> q):
    header(MqttType::SUBACK, false, 0, false, qos.size() + 2),
    msgId(m),
    qos(std::move(q)) {
}

void MqttSuback::cerealise(Cereal& cereal) {
    cereal.grain(header);
    cereal.grain(msgId);
    ushort size;
    cereal.grain(size);
    if(qos.size() != size) qos.resize(size);
    for(auto& q: qos) cereal.grain(q);
}


MqttUnsubscribe::MqttUnsubscribe(MqttFixedHeader h):header(h) {

}

void MqttUnsubscribe::handle(MqttServer& server, MqttConnection& connection) const {
    server.unsubscribe(connection, msgId, topics);
}

void MqttUnsubscribe::cerealise(Cereal& cereal) {
    cereal.grain(header);
    cereal.grain(msgId);
    ushort size;
    cereal.grain(size);
    if(topics.size() != size) topics.resize(size);
    for(auto& t: topics) cereal.grain(t);
}


MqttUnsuback::MqttUnsuback(ushort m):
    header(MqttType::UNSUBACK, false, 0, false, 2),
    msgId(m)
{
}

MqttUnsuback::MqttUnsuback(MqttFixedHeader h):header(h), msgId() {

}

void MqttUnsuback::cerealise(Cereal& cereal) {
    cereal.grain(header);
    cereal.grain(msgId);
}


void MqttDisconnect::handle(MqttServer& server, MqttConnection& connection) const {
    server.unsubscribe(connection);
    connection.disconnect();
}

void MqttPingReq::handle(MqttServer& server, MqttConnection& connection) const {
    server.ping(connection);
}

std::vector<ubyte> MqttPingResp::encode() const {
    return std::vector<ubyte>{0xd0, 0x00};
}
