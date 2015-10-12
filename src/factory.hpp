#ifndef FACTORY_H_
#define FACTORY_H_


#include "message.hpp"
#include <memory>


class MqttFactory {
public:
    static std::unique_ptr<MqttMessage> create(std::vector<ubyte> bytes);
    static void handleMessage(const std::vector<ubyte>& bytes, MqttServer& server, MqttConnection& connection);
};


#endif // FACTORY_H_
