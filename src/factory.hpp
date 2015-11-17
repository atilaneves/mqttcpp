#ifndef FACTORY_H_
#define FACTORY_H_


#include "message.hpp"
#include <memory>


class MqttFactory {
public:
    static std::unique_ptr<MqttMessage> create(std::vector<ubyte> bytes);
    static void handleMessage(std::vector<ubyte>::const_iterator begin, std::vector<ubyte>::const_iterator end,
                              OldMqttServer& server, OldMqttConnection& connection);
};


#endif // FACTORY_H_
