#ifndef BROKER_H_
#define BROKER_H_

#include "dtypes.hpp"
#include <vector>
#include <string>


class MqttSubscriber {
public:
    virtual void newMessage(std::string topic, std::vector<ubyte> payload) = 0;
};


#endif // BROKER_H_
