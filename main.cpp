#include "mqtt_tcp_server.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;


int main(int argc, char*[]) {
    cout << "C++ MQTT server" << endl;
    try {
        constexpr int port = 1883;
        const auto useCache = argc > 1;
        if(useCache) {
            cout << "Enabling the cache" << endl;
        } else {
            cout << "Disabling the cache" << endl;
        }
        MqttTcpServer server(port, useCache);
        server.run();
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
