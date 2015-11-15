#include "catch.hpp"
#include "broker.hpp"
#include "span.h"
#include <vector>

using namespace std;
using namespace gsl;


using Payload = vector<ubyte>;

struct TestMqttSubscriber {

    void newMessage(span<ubyte> bytes) {
        messages.emplace_back(bytes);
    }

    vector<Payload> messages;
};

TEST_CASE("foo") {
    for(const auto useCache: {false, true}) {
        MqttBroker<TestMqttSubscriber> broker{useCache};
        TestMqttSubscriber subscriber;

        vector<ubyte> msg1{2, 4, 6};
        vector<ubyte> msg2{1, 3, 5, 7};
        broker.publish("topics/foo", as_span(msg1));
        REQUIRE(subscriber.messages == vector<Payload>{});

        broker.subscribe(subscriber, {"topics/foo"});
        broker.publish("topics/foo", msg1);
        broker.publish("topics/bar", msg2);
        REQUIRE(subscriber.messages == vector<Payload>{msg1});

        broker.subscribe(subscriber, {"topics/bar"});
        broker.publish("topics/foo", msg1);
        broker.publish("topics/bar", msg2);
        REQUIRE(subscriber.messages == (vector<Payload>{msg1, msg1, msg2}));
    }
}
