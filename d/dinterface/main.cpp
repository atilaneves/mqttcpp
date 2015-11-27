#include "DlangServer.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;


int main(int argc, char*[]) {
    cout << "C++/D MQTT server" << endl;
    try {
        constexpr int port = 1883;
        const auto useCache = argc < 2;
        if(!useCache) {
            cout << "Disabling the cache" << endl;
        }
        startMqttServer(useCache);
        DlangServer server{port};
        server.run();
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
