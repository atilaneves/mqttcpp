#include "catch.hpp"
#include "broker.hpp"
#include "span.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace gsl;


using Payload = vector<ubyte>;

struct TestMqttSubscriber {

    void newMessage(span<ubyte> bytes) {
        messages.emplace_back(bytes.begin(), bytes.end());
        assert(messages.size() != 0);
        assert((long)messages[messages.size() - 1].size() == bytes.size());
        //debug();
    }

    void debug() {
        cout << "Messages: " << endl;
        for(const auto& msg: messages) {
            for(const auto b: msg) {
                printf("%#x, ", b);
            }
            cout << endl;
        }
    }

    vector<Payload> messages;
};

TEST_CASE("no subscriptions") {
    for(const auto useCache: {false, true}) {
        MqttBroker<TestMqttSubscriber> broker{useCache};
        TestMqttSubscriber subscriber;

        vector<ubyte> msg1{2, 4, 6};
        vector<ubyte> msg2{1, 3, 5, 7};

        broker.publish("topics/foo", as_span(msg1));
        broker.publish("topics/bar", as_span(msg2));

        REQUIRE(subscriber.messages == vector<Payload>{});
    }
}


TEST_CASE("subscribe") {
    for(const auto useCache: {false, true}) {
        MqttBroker<TestMqttSubscriber> broker{useCache};
        TestMqttSubscriber subscriber;

        vector<ubyte> msg1{2, 4, 6};
        vector<ubyte> msg2{1, 3, 5, 7};

        broker.subscribe(subscriber, {"topics/foo"});
        broker.publish(ensure_z("topics/foo"), msg1);
        broker.publish(ensure_z("topics/bar"), msg2);
        REQUIRE(subscriber.messages == vector<Payload>{msg1});

        broker.subscribe(subscriber, {"topics/bar"});
        broker.publish(ensure_z("topics/foo"), msg1);
        broker.publish(ensure_z("topics/bar"), msg2);
        REQUIRE(subscriber.messages == (vector<Payload>{msg1, msg1, msg2}));
    }
}
