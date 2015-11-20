#ifndef TESTCONNECTION_H_
#define TESTCONNECTION_H_


#include "message.hpp"
#include "gsl.h"
#include <vector>


using Payload = std::vector<ubyte>;


struct TestConnection {
    void newMessage(gsl::span<const ubyte> bytes);
    void disconnect();
    bool connected{false};
    MqttConnack::Code connectionCode{MqttConnack::Code::NO_AUTH};
    std::vector<Payload> payloads;
    Payload lastMsg;
};




#endif // TESTCONNECTION_H_
