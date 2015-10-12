#include "factory.hpp"
#include "Decerealiser.hpp"
#include <iostream>


namespace {
template<typename T, typename... A>
std::unique_ptr<T> make_unique(A... args) {
    return std::unique_ptr<T>(new T(args...));
}
} //anonymous namespace


std::unique_ptr<MqttMessage> MqttFactory::create(std::vector<ubyte> bytes) {
    Decerealiser cereal(bytes);
    auto fixedHeader = cereal.value<MqttFixedHeader>();
    // if(fixedHeader.remaining < cereal.getBytes().size()) {
    //     std::cerr << "Wrong MQTT remaining size " << (int)fixedHeader.remaining <<
    //         ". Real remaining size: " << cereal.getBytes().size() << std::endl;
    // }

    const auto mqttSize = fixedHeader.remaining + MqttFixedHeader::SIZE;
    if(mqttSize != bytes.size()) {
        std::cerr << "Malformed packet. Actual size: " << bytes.size() <<
            ". Advertised size: " << mqttSize <<
            " (r " << fixedHeader.remaining  << ")" << std::endl;
        return nullptr;
    }

    cereal.reset(); //so the messages created below can re-read the header

    switch(fixedHeader.type) {
    case MqttType::CONNECT:
        return cereal.createPtr<MqttConnect>(fixedHeader);
    case MqttType::CONNACK:
        return cereal.createPtr<MqttConnack>();
    case MqttType::PUBLISH:
        return cereal.createPtr<MqttPublish>(fixedHeader);
    case MqttType::SUBSCRIBE:
        if(fixedHeader.qos != 1) {
            std::cerr << "SUBSCRIBE message with qos " << fixedHeader.qos <<  ", should be 1" << std::endl;
        }
        return cereal.createPtr<MqttSubscribe>(fixedHeader);
    case MqttType::SUBACK:
        return cereal.createPtr<MqttSuback>(fixedHeader);
    case MqttType::UNSUBSCRIBE:
        return cereal.createPtr<MqttUnsubscribe>(fixedHeader);
    case MqttType::UNSUBACK:
        return cereal.createPtr<MqttUnsuback>(fixedHeader);
    case MqttType::PINGREQ:
        return make_unique<MqttPingReq>();
    case MqttType::PINGRESP:
        return make_unique<MqttPingResp>();
    case MqttType::DISCONNECT:
        return make_unique<MqttDisconnect>();
    default:
        std::cerr << "Unknown MQTT message type: " << (int)fixedHeader.type << std::endl;
        return nullptr;
    }
}

void MqttFactory::handleMessage(std::vector<ubyte>::const_iterator begin, std::vector<ubyte>::const_iterator end,
                                MqttServer& server, MqttConnection& connection) {

    Decerealiser cereal(begin, end);
    auto fixedHeader = cereal.value<MqttFixedHeader>();

    const auto mqttSize = fixedHeader.remaining + MqttFixedHeader::SIZE;
    if(mqttSize != (end - begin)) {
        std::cerr << "Malformed packet. Actual size: " << (end - begin) <<
            ". Advertised size: " << mqttSize <<
            " (r " << fixedHeader.remaining  << ")" << std::endl;
        return;
    }

    cereal.reset(); //so the messages created below can re-read the header

    switch(fixedHeader.type) {
    case MqttType::CONNECT:
        cereal.create<MqttConnect>(fixedHeader).handle(server, connection);
        break;
    case MqttType::CONNACK:
        cereal.create<MqttConnack>().handle(server, connection);
        break;
    case MqttType::PUBLISH:
        cereal.create<MqttPublish>(fixedHeader).handle(server, connection);
        break;
    case MqttType::SUBSCRIBE:
        if(fixedHeader.qos != 1) {
            std::cerr << "SUBSCRIBE message with qos " << fixedHeader.qos <<  ", should be 1" << std::endl;
        }
        cereal.create<MqttSubscribe>(fixedHeader).handle(server, connection);
        break;
    case MqttType::SUBACK:
        cereal.create<MqttSuback>(fixedHeader).handle(server, connection);
        break;
    case MqttType::UNSUBSCRIBE:
        cereal.create<MqttUnsubscribe>(fixedHeader).handle(server, connection);
        break;
    case MqttType::UNSUBACK:
        cereal.create<MqttUnsuback>(fixedHeader).handle(server, connection);
        break;
    case MqttType::PINGREQ:
        MqttPingReq().handle(server, connection);
        break;
    case MqttType::PINGRESP:
        MqttPingResp().handle(server, connection);
        break;
    case MqttType::DISCONNECT:
        MqttDisconnect().handle(server, connection);
        break;
    default:
        std::cerr << "Unknown MQTT message type: " << (int)fixedHeader.type << std::endl;
    }
}
