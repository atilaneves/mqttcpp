#include "broker.hpp"
#include "message.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <unordered_map>


namespace {

bool revStrEquals(const std::string& str1, const std::string& str2) { //compare strings in reverse
    if(str1.length() != str2.length()) return false;
    for(int i = str1.length() - 1; i >= 0; --i)
        if(str1[i] != str2[i]) return false;
    return true;
}

bool equalOrPlus(const std::string& pat, const std::string& top) {
    return pat == "+" || revStrEquals(pat, top);
}
} //anonymous namespace


class Subscription {
public:

    friend class SubscriptionTree;

    Subscription(MqttSubscriber& subscriber, MqttSubscribe::Topic topic,
                 std::deque<std::string> topicParts):
        _subscriber(subscriber),
        _part(std::move(topicParts.back())),
        _topic(std::move(topic.topic)),
        _qos(topic.qos) {
    }

    void newMessage(std::string topic, std::vector<ubyte> payload) {
        _subscriber.newMessage(topic, payload);
    }

    bool isSubscriber(const MqttSubscriber& subscriber) const {
        return &_subscriber == &subscriber;
    }

    bool isSubscription(const MqttSubscriber& subscriber,
                        std::vector<std::string> topics) const {
        return isSubscriber(subscriber) && isTopic(topics);
    }

    bool isTopic(std::vector<std::string> topics) const {
        return std::find(topics.cbegin(), topics.cend(), _topic) == topics.cend();
    }

private:
    MqttSubscriber& _subscriber;
    std::string _part;
    std::string _topic;
    ubyte _qos;
};



struct SubscriptionTree {
private:
    struct Node {
        Node(std::string pt, Node* pr):part(pt), parent(pr) {}
        std::string part;
        Node* parent;
        std::unordered_map<std::string, Node*> branches;
        std::vector<Subscription*> leaves;
    };


public:

    void addSubscription(Subscription* s, std::deque<std::string> parts) {
        assert(parts.size());
        clearCache();
        addSubscriptionImpl(s, parts, nullptr, _nodes);
    }

    void removeSubscription(MqttSubscriber& subscriber,
                            std::unordered_map<std::string, Node*>& nodes) {
        clearCache();
        std::unordered_map<std::string, Node*> newNodes = nodes;
        for(auto n: newNodes) {
            if(n.second->leaves.size()) {
                std::remove_if(n.second->leaves.begin(), n.second->leaves.end(),
                               [&subscriber](Subscription* l) { return l->isSubscriber(subscriber); });
                if(!n.second->leaves.size() && !n.second->branches.size()) {
                    removeNode(n.second->parent, n.second);
                }
            } else {
                removeSubscription(subscriber, n.second->branches);
            }
        }
    }

    void removeSubscription(MqttSubscriber& subscriber,
                            std::vector<std::string> topic,
                            std::unordered_map<std::string, Node*>& nodes) {
        clearCache();
        std::unordered_map<std::string, Node*> newNodes = nodes;
        for(auto n: newNodes) {
            if(n.second->leaves.size()) {
                std::remove_if(n.second->leaves.begin(), n.second->leaves.end(),
                               [&subscriber, &topic](Subscription* l) {
                                   return l->isSubscription(subscriber, topic); });
                if(!n.second->leaves.size() && !n.second->branches.size()) {
                    removeNode(n.second->parent, n.second);
                }
            } else {
                removeSubscription(subscriber, topic, n.second->branches);
            }
        }
    }


    void publish(std::string topic, std::deque<std::string> topParts,
                 std::vector<ubyte> payload) {
        publish(topic, topParts, payload, _nodes);
    }

    void publish(std::string topic, std::deque<std::string> topParts,
                 std::vector<ubyte> payload,
                 std::unordered_map<std::string, Node*> nodes) {
        //check the cache first
        if(_useCache && _cache.count(topic)) {
            for(auto s: _cache[topic]) s->newMessage(topic, payload);
            return;
        }

        //not in the cache or not using the cache, do it the hard way
        for(auto part: std::vector<std::string>{topParts[0], "#", "+"}) {
            if(nodes.count(part)) {
                if(topParts.size() == 1 && nodes[part]->branches.count("#")) {
                    //So that "finance/#" matches finance
                    publishLeaves(topic, payload, topParts, nodes[part]->branches["#"]->leaves);
                }
                publishLeaves(topic, payload, topParts, nodes[part]->leaves);
                if(topParts.size() > 1) {
                    topParts.pop_front();
                    publish(topic, topParts, payload, nodes[part]->branches);
                }
            }
        }
    }

    void publishLeaves(std::string topic, std::vector<ubyte> payload,
                       std::deque<std::string> topParts,
                       std::vector<Subscription*> subscriptions) {
        for(auto sub: subscriptions) {
            if(topParts.size() == 1 &&
               equalOrPlus(sub->_part, topParts[0])) {
                publishLeaf(sub, topic, payload);
            }
            else if(sub->_part == "#") {
                publishLeaf(sub, topic, payload);
            }
        }
    }

    void publishLeaf(Subscription* sub, std::string topic, std::vector<ubyte> payload) {
        sub->newMessage(topic, payload);
        if(_useCache) _cache[topic].push_back(sub);
    }

    void useCache(bool u) { _useCache = u; }

private:

    bool _useCache;
    std::unordered_map<std::string, std::vector<Subscription*>> _cache;
    std::unordered_map<std::string, Node*> _nodes;
    friend class MqttBroker;

    void addSubscriptionImpl(Subscription* s,
                             std::deque<std::string> parts,
                             Node* parent,
                             std::unordered_map<std::string, Node*> nodes) {
        auto part = parts.front();
        parts.pop_front();
        auto node = addOrFindNode(part, parent, nodes);
        if(!parts.size()) {
            node->leaves.emplace_back(s);
        } else {
            addSubscriptionImpl(s, parts, node, node->branches);
        }
    }


    Node* addOrFindNode(std::string part, Node* parent,
                        std::unordered_map<std::string, Node*> nodes) {
        if(nodes.count(part) && part == nodes[part]->part) {
            return nodes[part];
        }
        auto node = new Node(part, parent);
        nodes[part] = node;
        return node;
    }

    void clearCache() {
        if(_useCache) _cache.clear();
    }

    void removeNode(Node* parent, Node* child) {
        if(parent) {
            parent->branches.erase(child->part);
        } else {
            _nodes.erase(child->part);
        }
        if(parent && !parent->branches.size() && !parent->leaves.size())
            removeNode(parent->parent, parent);
    }
};



class MqttBroker {
public:

    void subscribe(MqttSubscriber& subscriber, std::vector<std::string> topics) {
        std::vector<MqttSubscribe::Topic> newTopics;
        std::transform(topics.cbegin(), topics.cend(), std::back_inserter(newTopics),
                       [](std::string t) { return MqttSubscribe::Topic(t, 0); });
        subscribe(subscriber, newTopics);
    }

    void subscribe(MqttSubscriber& subscriber, std::vector<MqttSubscribe::Topic> topics) {
        for(const auto& t: topics) {
            std::deque<std::string> parts;
            boost::split(parts, t.topic, boost::is_any_of("/"));
            _subscriptions.addSubscription(new Subscription(subscriber, t, parts), parts);
        }
    }

    void unsubscribe(MqttSubscriber& subscriber) {
        _subscriptions.removeSubscription(subscriber, _subscriptions._nodes);
    }

    void unsubscribe(MqttSubscriber& subscriber, std::vector<std::string> topics) {
        _subscriptions.removeSubscription(subscriber, topics, _subscriptions._nodes);
    }

    void publish(std::string topic, std::vector<ubyte> payload) {
        std::deque<std::string> topParts;
        boost::split(topParts, topic, boost::is_any_of("/"));
        publish(topic, topParts, payload);
    }

    void useCache(bool u) { _subscriptions.useCache(u); }

private:

    SubscriptionTree _subscriptions;

    void publish(std::string topic, std::deque<std::string> topParts,
                 std::vector<ubyte> payload) {
        _subscriptions.publish(topic, topParts, payload);
    }
};
