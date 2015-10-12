#include "unit_thread.hpp"
#include "broker.hpp"


struct TestMqttSubscriber: public MqttSubscriber {
    void newMessage(std::string topic, const std::vector<ubyte>& payload) override {
        (void)topic;
        std::string strPayload(payload.begin(), payload.end());
        messages.emplace_back(strPayload);
    }
    std::vector<std::string> messages;
};


struct Subscribe: public TestCase {
    void test() override {
        auto broker = MqttBroker();

        TestMqttSubscriber subscriber;
        broker.publish("topics/foo", "my foo is foo");
        checkEqual(subscriber.messages, std::vector<std::string>({}));

        broker.subscribe(subscriber, std::vector<std::string>({"topics/foo"}));
        broker.publish("topics/foo", "my foo is foo");
        broker.publish("topics/bar", "my bar is bar");
        checkEqual(subscriber.messages, std::vector<std::string>({"my foo is foo"}));

        broker.subscribe(subscriber, std::vector<std::string>({"topics/bar"}));
        broker.publish("topics/foo", "my foo is foo");
        broker.publish("topics/bar", "my bar is bar");
        checkEqual(subscriber.messages,
                   std::vector<std::string>({"my foo is foo", "my foo is foo", "my bar is bar"}));
    }
};
REGISTER_TEST(broker, Subscribe)



struct UnsubscribeAll: public TestCase {
    void test() override {
        auto broker = MqttBroker();
        TestMqttSubscriber subscriber;

        broker.subscribe(subscriber, std::vector<std::string>({"topics/foo"}));
        broker.publish("topics/foo", "my foo is foo");
        broker.publish("topics/bar", "my bar is bar");
        checkEqual(subscriber.messages, std::vector<std::string>({"my foo is foo"}));

        broker.unsubscribe(subscriber);
        broker.publish("topics/foo", "my foo is foo");
        broker.publish("topics/bar", "my bar is bar");
        checkEqual(subscriber.messages, std::vector<std::string>({"my foo is foo"})); //shouldn't have changed
    }
};
REGISTER_TEST(broker, UnsubscribeAll)



struct UnsubscribeOne: public TestCase {
    void test() override {
        auto broker = MqttBroker();
        TestMqttSubscriber subscriber;

        broker.subscribe(subscriber, std::vector<std::string>({"topics/foo", "topics/bar"}));
        broker.publish("topics/foo", "my foo is foo");
        broker.publish("topics/bar", "my bar is bar");
        broker.publish("topics/baz", "my baz is baz");
        checkEqual(subscriber.messages, std::vector<std::string>({"my foo is foo", "my bar is bar"}));

        broker.unsubscribe(subscriber, std::vector<std::string>({"topics/foo"}));
        broker.publish("topics/foo", "my foo is foo");
        broker.publish("topics/bar", "my bar is bar");
        broker.publish("topics/baz", "my baz is baz");
        checkEqual(subscriber.messages,
                   std::vector<std::string>({"my foo is foo", "my bar is bar", "my bar is bar"}));
    }
};
REGISTER_TEST(broker, UnsubscribeOne)


struct WildCards: public TestCase {
    void test() override {
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
        checkMatches("topics/dir/foo", "#", true);
    }

    void checkMatches(std::string pubTopic, std::string subTopic, bool matches) {
        MqttBroker broker;
        TestMqttSubscriber subscriber;

        broker.subscribe(subscriber, std::vector<std::string>({subTopic}));
        broker.publish(pubTopic, "payload");
        const auto expected = matches ? std::vector<std::string>({"payload"}) : std::vector<std::string>{};
        checkEqual(subscriber.messages, expected);
    }

};
REGISTER_TEST(broker, WildCards)


struct SubscribeWithWildCards: public TestCase {
    void test() override {
        MqttBroker broker;
        TestMqttSubscriber subscriber1;

        broker.subscribe(subscriber1, std::vector<std::string>({"topics/foo/+"}));
        broker.publish("topics/foo/bar", "3");
        broker.publish("topics/bar/baz/boo", "4"); //shouldn't get this one
        checkEqual(subscriber1.messages, std::vector<std::string>({"3"}));

        TestMqttSubscriber subscriber2;
        broker.subscribe(subscriber2, std::vector<std::string>({"topics/foo/#"}));
        broker.publish("topics/foo/bar", "3");
        broker.publish("topics/bar/baz/boo", "4");

        checkEqual(subscriber1.messages, std::vector<std::string>({"3", "3"}));
        checkEqual(subscriber2.messages, std::vector<std::string>({"3"}));

        TestMqttSubscriber subscriber3;
        broker.subscribe(subscriber3, std::vector<std::string>({"topics/+/bar"}));
        TestMqttSubscriber subscriber4;
        broker.subscribe(subscriber4, std::vector<std::string>({"topics/#"}));

        broker.publish("topics/foo/bar", "3");
        broker.publish("topics/bar/baz/boo", "4");
        broker.publish("topics/boo/bar/zoo", "5");
        broker.publish("topics/foo/bar/zoo", "6");
        broker.publish("topics/bbobobobo/bar", "7");

        checkEqual(subscriber1.messages, std::vector<std::string>({"3", "3", "3"}));
        checkEqual(subscriber2.messages, std::vector<std::string>({"3", "3", "6"}));
        checkEqual(subscriber3.messages, std::vector<std::string>({"3", "7"}));
        checkEqual(subscriber4.messages, std::vector<std::string>({"3", "4", "5", "6", "7"}));
    }
};
REGISTER_TEST(broker, SubscribeWithWildCards)
