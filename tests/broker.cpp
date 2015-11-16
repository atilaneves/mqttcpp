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


TEST_CASE("unsubscribe all") {
    for(const auto useCache: {false, true}) {
        MqttBroker<TestMqttSubscriber> broker{useCache};
        TestMqttSubscriber subscriber;

        vector<ubyte> msg1{2, 4, 6};
        vector<ubyte> msg2{1, 3, 5, 7};

        broker.subscribe(subscriber, {"topics/foo"});
        broker.publish(ensure_z("topics/foo"), msg1);
        broker.publish(ensure_z("topics/bar"), msg2);
        REQUIRE(subscriber.messages == vector<Payload>{msg1});

        broker.unsubscribe(subscriber);
        broker.publish(ensure_z("topics/foo"), msg1);
        broker.publish(ensure_z("topics/bar"), msg2);
        REQUIRE(subscriber.messages == vector<Payload>{msg1}); //shouldn't have changed
    }
}

TEST_CASE("unsubscribe one") {
    for(const auto useCache: {false, true}) {
        MqttBroker<TestMqttSubscriber> broker{useCache};
        TestMqttSubscriber subscriber;

        vector<ubyte> msg1{2, 4, 6};
        vector<ubyte> msg2{1, 3, 5, 7};
        vector<ubyte> msg3{9, 8, 7, 6, 5};

        broker.subscribe(subscriber, vector<string>{"topics/foo", "topics/bar"});
        broker.publish(ensure_z("topics/foo"), msg1);
        broker.publish(ensure_z("topics/bar"), msg2);
        broker.publish(ensure_z("topics/baz"), msg3);
        REQUIRE(subscriber.messages == (vector<Payload>{msg1, msg2}));

        broker.unsubscribe(subscriber, {"topics/foo"});
        broker.publish(ensure_z("topics/foo"), msg1);
        broker.publish(ensure_z("topics/bar"), msg2);
        broker.publish(ensure_z("topics/baz"), msg3);
        REQUIRE(subscriber.messages == (vector<Payload>{msg1, msg2, msg2}));
    }
}

static void checkMatches(const std::string& pubTopic, const std::string& subTopic, bool matches) {
    for(const auto useCache: {false, true}) {
        MqttBroker<TestMqttSubscriber> broker{useCache};
        TestMqttSubscriber subscriber;

        vector<ubyte> msg1{2, 4, 6};

        broker.subscribe(subscriber, {subTopic});
        broker.publish(pubTopic, msg1);
        if(matches)
            REQUIRE(subscriber.messages == vector<Payload>{msg1});
        else
            REQUIRE(subscriber.messages == vector<Payload>{});
    }
}

TEST_CASE("wildcards match") {
    checkMatches("foo/bar/baz", "foo/bar/baz", true);
    checkMatches("foo/bar", "foo/+", true);
    checkMatches("foo/baz", "foo/+", true);
    checkMatches("foo/bar/baz", "foo/+", false);
    checkMatches("foo/bar", "foo/#", true);
    checkMatches("foo/bar/baz", "foo/#", true);
    checkMatches("foo/bar/baz/boo", "foo/#", true);
    checkMatches("foo/bla/bar/baz/boo/bogadog", "foo/+/bar/baz/#", true);
    checkMatches("finance", "finance/#", true);
    checkMatches("finance", "finance#", false);
    checkMatches("finance", "#", true);
    checkMatches("finance/stock", "#", true);
    checkMatches("finance/stock", "finance/stock/ibm", false);
    checkMatches("topics/foo/bar", "topics/foo/#", true);
    checkMatches("topics/bar/baz/boo", "topics/foo/#", false);
}

TEST_CASE("subscribe with wildcards") {
    for(const auto useCache: {false, true}) {
        vector<ubyte> msg3{3};
        vector<ubyte> msg4{4};
        vector<ubyte> msg5{5};
        vector<ubyte> msg6{6};
        vector<ubyte> msg7{7};

        MqttBroker<TestMqttSubscriber> broker{useCache};
        TestMqttSubscriber subscriber1;

        broker.subscribe(subscriber1, {"topics/foo/+"});
        broker.publish("topics/foo/bar", msg3);
        broker.publish("topics/bar/baz/boo", msg4); //shouldn't get this one
        REQUIRE(subscriber1.messages == vector<Payload>{msg3});

        TestMqttSubscriber subscriber2;
        broker.subscribe(subscriber2, {"topics/foo/#"});
        broker.publish("topics/foo/bar", msg3);
        broker.publish("topics/bar/baz/boo", msg4);

        REQUIRE(subscriber1.messages == (vector<Payload>{msg3, msg3}));
        REQUIRE(subscriber2.messages == (vector<Payload>{msg3}));

        TestMqttSubscriber subscriber3;
        broker.subscribe(subscriber3, {"topics/+/bar"});
        TestMqttSubscriber subscriber4;
        broker.subscribe(subscriber4, {"topics/#"});

        broker.publish("topics/foo/bar", msg3);
        broker.publish("topics/bar/baz/boo", msg4);
        broker.publish("topics/boo/bar/zoo", msg5);
        broker.publish("topics/foo/bar/zoo", msg6);
        broker.publish("topics/bbobobobo/bar", msg7);

        REQUIRE(subscriber1.messages == (vector<Payload>{msg3, msg3, msg3}));
        REQUIRE(subscriber2.messages == (vector<Payload>{msg3, msg3, msg6}));
        REQUIRE(subscriber3.messages == (vector<Payload>{msg3, msg7}));
        REQUIRE(subscriber4.messages == (vector<Payload>{msg3, msg4, msg5, msg6, msg7}));

    }
}
