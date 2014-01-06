#ifndef BROKER_H_
#define BROKER_H_

#include "dtypes.hpp"
#include "message.hpp"
#include <vector>
#include <deque>
#include <string>
#include <unordered_map>


class MqttSubscriber {
public:
    virtual void newMessage(std::string topic, std::vector<ubyte> payload) = 0;
};


class Subscription {
public:

    friend class SubscriptionTree;

    Subscription(MqttSubscriber& subscriber, MqttSubscribe::Topic topic,
                 std::deque<std::string> topicParts);

    void newMessage(std::string topic, std::vector<ubyte> payload);

    bool isSubscriber(const MqttSubscriber& subscriber) const;
    bool isSubscription(const MqttSubscriber& subscriber,
                        std::vector<std::string> topics) const;
    bool isTopic(std::vector<std::string> topics) const;

private:
    MqttSubscriber& _subscriber;
    std::string _part;
    std::string _topic;
    ubyte _qos;
};


struct SubscriptionTree {
private:

    struct Node {
        using NodePtr = Node*;
        Node(std::string pt, NodePtr pr):part(pt), parent(pr) {}
        std::string part;
        NodePtr parent;
        std::unordered_map<std::string, NodePtr> branches;
        std::vector<Subscription*> leaves;
    };

public:

    using NodePtr = Node::NodePtr;

    void addSubscription(Subscription* s, std::deque<std::string> parts);
    void removeSubscription(MqttSubscriber& subscriber,
                            std::unordered_map<std::string, NodePtr>& nodes);
    void removeSubscription(MqttSubscriber& subscriber,
                            std::vector<std::string> topic,
                            std::unordered_map<std::string, NodePtr>& nodes);
    void publish(std::string topic, std::deque<std::string> topParts,
                 std::vector<ubyte> payload);
    void publish(std::string topic, std::deque<std::string> topParts,
                 std::vector<ubyte> payload,
                 std::unordered_map<std::string, NodePtr>& nodes);
    void publishLeaves(std::string topic, std::vector<ubyte> payload,
                       std::deque<std::string> topParts,
                       std::vector<Subscription*> subscriptions);
    void publishLeaf(Subscription* sub, std::string topic, std::vector<ubyte> payload);
    void useCache(bool u) { _useCache = u; }

private:

    bool _useCache;
    std::unordered_map<std::string, std::vector<Subscription*>> _cache;
    std::unordered_map<std::string, NodePtr> _nodes;
    friend class MqttBroker;

    void addSubscriptionImpl(Subscription* s,
                             std::deque<std::string> parts,
                             NodePtr parent,
                             std::unordered_map<std::string, NodePtr>& nodes);
    NodePtr addOrFindNode(std::string part, NodePtr parent,
                        std::unordered_map<std::string, NodePtr>& nodes);
    void clearCache() { if(_useCache) _cache.clear(); }

    void removeNode(NodePtr parent, NodePtr child);
};



class MqttBroker {
public:

    void subscribe(MqttSubscriber& subscriber, std::vector<std::string> topics);
    void subscribe(MqttSubscriber& subscriber, std::vector<MqttSubscribe::Topic> topics);
    void unsubscribe(MqttSubscriber& subscriber);
    void unsubscribe(MqttSubscriber& subscriber, std::vector<std::string> topics);
    void publish(std::string topic, std::string payload);
    void publish(std::string topic, std::vector<ubyte> payload);

    void useCache(bool u) { _subscriptions.useCache(u); }

private:

    SubscriptionTree _subscriptions;

    void publish(std::string topic, std::deque<std::string> topParts,
                 std::vector<ubyte> payload);
};

using NodePtr = SubscriptionTree::NodePtr;

#endif // BROKER_H_
