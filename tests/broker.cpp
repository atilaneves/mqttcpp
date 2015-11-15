#include "catch.hpp"
#include "broker.hpp"
#include "span.h"
#include <vector>

using namespace std;
using namespace gsl;


struct TestMqttSubscriber {
    using Payload = vector<ubyte>;

    void newMessage(span<ubyte> bytes) {
        messages.emplace_back(bytes);
    }

    vector<Payload> messages;
};

TEST_CASE("foo") {
    for(const auto useCache: {false, true}) {
        MqttBroker<TestMqttSubscriber> broker{useCache};
    }
}
