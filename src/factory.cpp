#include "message.hpp"
#include "Decerealiser.hpp"
#include <iostream>
#include <memory>

template<typename T, typename... A>
std::unique_ptr<T> make_unique(A... args) {
    return std::unique_ptr<T>(new T(args...));
}

class MqttFactory {
public:
    static std::unique_ptr<MqttMessage> create(std::vector<ubyte> bytes) {
        Decerealiser cereal(bytes);
        auto fixedHeader = cereal.value<MqttFixedHeader>();
        if(fixedHeader.remaining < cereal.getBytes().size()) {
            std::cerr << "Wrong MQTT remaining size " << (int)fixedHeader.remaining <<
                ". Real remaining size: " << cereal.getBytes().size() << std::endl;
        }

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
            return cereal.create<MqttConnect>(fixedHeader);
        case MqttType::CONNACK:
            return cereal.create<MqttConnack>();
        case MqttType::PUBLISH:
            return cereal.create<MqttPublish>(fixedHeader);
        case MqttType::SUBSCRIBE:
            if(fixedHeader.qos != 1) {
                std::cerr << "SUBSCRIBE message with qos " << fixedHeader.qos <<  ", should be 1" << std::endl;
            }
            return cereal.create<MqttSubscribe>(fixedHeader);
        case MqttType::SUBACK:
            return cereal.create<MqttSuback>(fixedHeader);
        case MqttType::UNSUBSCRIBE:
            return cereal.create<MqttUnsubscribe>(fixedHeader);
        case MqttType::UNSUBACK:
            return cereal.create<MqttUnsuback>(fixedHeader);
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
};
