#ifndef SERVER_H_
#define SERVER_H_

#include "dtypes.hpp"
#include "message.hpp"
#include "broker.hpp"
#include "Decerealiser.hpp"
#include "gsl.h"
#include <string>
#include <vector>



template<typename C>
class MqttServer {
public:

    MqttServer(bool useCache = false):
        _broker{useCache}
    {
    }

    void newMessage(C& connection, gsl::span<const ubyte> bytes) {

        const auto type = getMessageType(bytes);
        switch(type) {
        case MqttType::CONNECT:
            static ubyte connackOk[] = {32, 2, 0, 0};
            //TODO: return something other than ok
            connection.newMessage(connackOk);
            break;

        case MqttType::PINGREQ:
            static ubyte pingResp[] = {0xd0, 0};
            connection.newMessage(pingResp);
            break;

        case MqttType::PUBLISH:
            {
                _broker.publish(getPublishTopic(bytes), bytes);
            }
            break;

        case MqttType::SUBSCRIBE:
            {
                Decerealiser dec{bytes};
                const auto hdr = dec.create<MqttFixedHeader>();
                dec.reset();
                const auto msg = dec.create<MqttSubscribe>(hdr);

                _broker.subscribe(connection, msg.topics);

                std::vector<ubyte> suback{0x90, 3, 0, 0, 0};
                suback[2] = msg.msgId >> 8;
                suback[3] = msg.msgId & 0xff;
                connection.newMessage(suback);
            }
            break;

        case MqttType::UNSUBSCRIBE:
            {
                Decerealiser dec{bytes};
                const auto hdr = dec.create<MqttFixedHeader>();
                dec.reset();
                const auto msg = dec.create<MqttUnsubscribe>(hdr);
                _broker.unsubscribe(connection, msg.topics);
                const std::vector<ubyte> unsuback{
                    0xb0, 2,
                        static_cast<ubyte>(msg.msgId >> 8), static_cast<ubyte>(msg.msgId & 0xff)};
                connection.newMessage(unsuback);
            }
            break;

        case MqttType::DISCONNECT:
            _broker.unsubscribe(connection);
            connection.disconnect();
            break;

        default:
            std::cerr << "Unknown message type " << (int)type << ":" << std::endl;
            std::cerr << "[";
            for(const int b: bytes) std::cerr << b << ", ";
            std::cerr << "]" << std::endl;
            break;
        }
    }

private:

    MqttBroker<C> _broker;
};

#endif // SERVER_H_
