#ifndef SERVER_H_
#define SERVER_H_

#include "dtypes.hpp"
#include <string>
#include <vector>


class MqttServer {
public:

    void publish(std::string topic, std::vector<ubyte> payload);
};



#endif // SERVER_H_
