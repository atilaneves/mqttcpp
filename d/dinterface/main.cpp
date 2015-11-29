#include "DlangServer.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;

extern "C" {
    int rt_init();
    int rt_term();
}


int main(int argc, char*[]) {
    cout << "C++/D MQTT server" << endl;
    rt_init();
    try {
        constexpr int port = 1883;
        const auto useCache = argc > 1;
        if(useCache) {
            cout << "Enabling the cache" << endl;
        } else {
            cout << "Disabling the cache" << endl;
        }
        startMqttServer(useCache);
        DlangServer server{port};
        server.run();
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    rt_term();
    return 0;
}
