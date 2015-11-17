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


class OldMqttConnection: public MqttSubscriber {
public:
    virtual void newMessage(const std::string& topic, const std::vector<ubyte>& payload) override;
    virtual void write(const std::vector<ubyte>& bytes) = 0;
    virtual void disconnect() = 0;
};


class OldMqttServer {
public:

    void newConnection(OldMqttConnection& connection,
                       const MqttConnect* connect);
    void subscribe(OldMqttConnection& connection, ushort msgId,
                   std::vector<std::string> topics);
    void subscribe(OldMqttConnection& connection, ushort msgId,
                   std::vector<MqttSubscribe::Topic> topics);
    void unsubscribe(OldMqttConnection& connection);
    void unsubscribe(OldMqttConnection& connection, ushort msgId,
                     std::vector<std::string> topics);
    void publish(const std::string& topic, const std::string& payload);
    void publish(const std::string& topic, const std::vector<ubyte>& payload);
    void ping(OldMqttConnection& connection);
    void useCache(bool u) { _broker.useCache(u); }

private:

    OldMqttBroker _broker;
};


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

        default:
            throw std::runtime_error("Unknown message type");
        }
    }

private:

    MqttBroker<C> _broker;
};

#endif // SERVER_H_
