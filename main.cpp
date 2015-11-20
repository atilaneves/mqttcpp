#include "mqtt_tcp_server.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;


int main() {
    cout << "C++ MQTT server" << endl;
    try {
        constexpr int port = 1883;
        MqttTcpServer server(port);
        server.run();
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
