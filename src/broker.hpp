#ifndef BROKER_H_
#define BROKER_H_

#include "dtypes.hpp"
#include "message.hpp"
#include <vector>
#include <deque>
#include <string>
#include <unordered_map>
#include <memory>

class MqttSubscriber {
public:
    virtual void newMessage(const std::string& topic, const std::vector<ubyte>& payload) = 0;
};

using TopicParts = std::deque<std::string>;

class Subscription {
public:

    friend class SubscriptionTree;

    Subscription(MqttSubscriber& subscriber, MqttSubscribe::Topic topic,
                 TopicParts topicParts);

    void newMessage(const std::string& topic, const std::vector<ubyte>& payload);

    bool isSubscriber(const MqttSubscriber& subscriber) const;
    bool isSubscription(const MqttSubscriber& subscriber,
                        std::vector<std::string> topics) const;
    bool isTopic(const std::vector<std::string>& topics) const;

    ubyte qos() const { return _qos; }

private:
    MqttSubscriber& _subscriber;
    std::string _part;
    std::string _topic;
    ubyte _qos;
};


class SubscriptionTree {
private:

    struct Node {
        using NodePtr = Node*;
        Node(std::string pt, NodePtr pr):part(pt), parent(pr) {}
        ~Node() { for(auto l: leaves) delete l; }
        std::string part;
        NodePtr parent;
        std::unordered_map<std::string, NodePtr> branches;
        std::vector<Subscription*> leaves;
    };

public:

    using NodePtr = Node::NodePtr;

    SubscriptionTree();

    void addSubscription(Subscription* s, const TopicParts& parts);
    void removeSubscription(MqttSubscriber& subscriber,
                            std::unordered_map<std::string, NodePtr>& nodes);
    void removeSubscription(MqttSubscriber& subscriber,
                            std::vector<std::string> topic,
                            std::unordered_map<std::string, NodePtr>& nodes);
    void publish(std::string topic, TopicParts topParts,
                 const std::vector<ubyte>& payload);
    void publish(std::string topic,
                 TopicParts::const_iterator topPartsBegin,
                 TopicParts::const_iterator topPartsEnd,
                 const std::vector<ubyte>& payload,
                 std::unordered_map<std::string, NodePtr>& nodes);
    void publishLeaves(std::string topic, const std::vector<ubyte>& payload,
                       TopicParts::const_iterator topPartsBegin,
                       TopicParts::const_iterator topPartsEnd,
                       std::vector<Subscription*> subscriptions);
    void publishLeaf(Subscription* sub, std::string topic, const std::vector<ubyte>& payload);
    void useCache(bool u) { _useCache = u; }

    bool _useCache;
    std::unordered_map<std::string, std::vector<Subscription*>> _cache;
    std::unordered_map<std::string, NodePtr> _nodes;
    friend class OldMqttBroker;

    void addSubscriptionImpl(Subscription* s,
                             TopicParts parts,
                             NodePtr parent,
                             std::unordered_map<std::string, NodePtr>& nodes);
    NodePtr addOrFindNode(std::string part, NodePtr parent,
                        std::unordered_map<std::string, NodePtr>& nodes);
    void clearCache() { if(_useCache) _cache.clear(); }

    void removeNode(NodePtr parent, NodePtr child);
};



class OldMqttBroker {
public:

    void subscribe(MqttSubscriber& subscriber, std::vector<std::string> topics);
    void subscribe(MqttSubscriber& subscriber, std::vector<MqttSubscribe::Topic> topics);
    void unsubscribe(MqttSubscriber& subscriber);
    void unsubscribe(MqttSubscriber& subscriber, std::vector<std::string> topics);
    void publish(const std::string& topic, const std::string& payload);
    void publish(const std::string& topic, const std::vector<ubyte>& payload);

    void useCache(bool u) { _subscriptions.useCache(u); }

private:

    SubscriptionTree _subscriptions;

    void publish(const std::string& topic,
                 const TopicParts& topParts,
                 const std::vector<ubyte>& payload);
};

using NodePtr = SubscriptionTree::NodePtr;

#include "string_span.h"

template<typename S>
class MqttBroker {
public:

    MqttBroker(bool useCache):
        _useCache{useCache}
    {
    }

    void publish(gsl::cstring_span<> topic, gsl::span<ubyte> bytes) {
        (void)topic;
        (void)bytes;
    }

    void subscribe(S& subscriber, std::vector<std::string> topics) {
        (void)subscriber;
        (void)topics;
    }

private:

    bool _useCache;
};

#endif // BROKER_H_
