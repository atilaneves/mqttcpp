#ifndef SERVER_H_
#define SERVER_H_

#include "dtypes.hpp"
#include "gsl.h"
#include "message.hpp"
#include "Decerealiser.hpp"
#include <stdexcept>

#include "broker.hpp"
#include <string>
#include <vector>



template<typename C>
class MqttServer {
public:

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
            Decerealiser dec{bytes};
            const auto hdr = dec.create<MqttFixedHeader>();
            dec.reset();
            const auto msg = dec.create<MqttPublish>(hdr);
            _broker.publish(msg.topic, bytes);
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
            connection.disconnect();
            _broker.unsubscribe(connection);
            break;

        default:
            throw std::runtime_error("Unknown message type");
        }
    }

private:

    MqttBroker<C> _broker;
};

#endif // SERVER_H_
