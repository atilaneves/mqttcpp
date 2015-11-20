#include "TestConnection.hpp"
#include "Decerealiser.hpp"


using namespace std;
using namespace gsl;


void TestConnection::newMessage(span<const ubyte> bytes) {
    switch(getMessageType(bytes)) {
    case MqttType::CONNACK:
        connected = true;
        connectionCode = MqttConnack::Code::ACCEPTED;
        break;

    case MqttType::PUBLISH:
        {
            Decerealiser dec{bytes};
            const auto hdr = dec.create<MqttFixedHeader>();
            dec.reset();
            const auto msg = dec.create<MqttPublish>(hdr);
            payloads.emplace_back(msg.payload.begin(), msg.payload.end());
        }
        break;
    default:
        break;
    }

    lastMsg = Payload(bytes.begin(), bytes.end());
}

void TestConnection::disconnect() {
    connected = false;
}
