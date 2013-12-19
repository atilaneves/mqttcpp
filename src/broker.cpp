#include "broker.hpp"
#include "message.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>


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


Subscription::Subscription(MqttSubscriber& subscriber, MqttSubscribe::Topic topic,
                           std::deque<std::string> topicParts):
    _subscriber(subscriber),
    _part(std::move(topicParts.back())),
    _topic(std::move(topic.topic)),
    _qos(topic.qos) {
}

void Subscription::newMessage(std::string topic, std::vector<ubyte> payload) {
    _subscriber.newMessage(topic, payload);
}

bool Subscription::isSubscriber(const MqttSubscriber& subscriber) const {
    return &_subscriber == &subscriber;
}

bool Subscription::isSubscription(const MqttSubscriber& subscriber,
                                  std::vector<std::string> topics) const {
    return isSubscriber(subscriber) && isTopic(topics);
}

bool Subscription::isTopic(std::vector<std::string> topics) const {
    return std::find(topics.cbegin(), topics.cend(), _topic) == topics.cend();
}
void SubscriptionTree::addSubscription(Subscription* s, std::deque<std::string> parts) {
    assert(parts.size());
    clearCache();
    addSubscriptionImpl(s, parts, nullptr, _nodes);
}

void SubscriptionTree::removeSubscription(MqttSubscriber& subscriber,
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

void SubscriptionTree::removeSubscription(MqttSubscriber& subscriber,
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


void SubscriptionTree::publish(std::string topic, std::deque<std::string> topParts,
                               std::vector<ubyte> payload) {
    publish(topic, topParts, payload, _nodes);
}

void SubscriptionTree::publish(std::string topic, std::deque<std::string> topParts,
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

void SubscriptionTree::publishLeaves(std::string topic, std::vector<ubyte> payload,
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

void SubscriptionTree::publishLeaf(Subscription* sub, std::string topic, std::vector<ubyte> payload) {
    sub->newMessage(topic, payload);
    if(_useCache) _cache[topic].push_back(sub);
}


void SubscriptionTree::addSubscriptionImpl(Subscription* s,
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


auto SubscriptionTree::addOrFindNode(std::string part, Node* parent,
                                      std::unordered_map<std::string, Node*> nodes) -> Node* {
    if(nodes.count(part) && part == nodes[part]->part) {
        return nodes[part];
    }
    auto node = new Node(part, parent);
    nodes[part] = node;
    return node;
}


void SubscriptionTree::removeNode(Node* parent, Node* child) {
    if(parent) {
        parent->branches.erase(child->part);
    } else {
        _nodes.erase(child->part);
    }
    if(parent && !parent->branches.size() && !parent->leaves.size())
        removeNode(parent->parent, parent);
}


void MqttBroker::subscribe(MqttSubscriber& subscriber, std::vector<std::string> topics) {
    std::vector<MqttSubscribe::Topic> newTopics;
    std::transform(topics.cbegin(), topics.cend(), std::back_inserter(newTopics),
                   [](std::string t) { return MqttSubscribe::Topic(t, 0); });
    subscribe(subscriber, newTopics);
}

void MqttBroker::subscribe(MqttSubscriber& subscriber, std::vector<MqttSubscribe::Topic> topics) {
    for(const auto& t: topics) {
        std::deque<std::string> parts;
        boost::split(parts, t.topic, boost::is_any_of("/"));
        _subscriptions.addSubscription(new Subscription(subscriber, t, parts), parts);
    }
}

void MqttBroker::unsubscribe(MqttSubscriber& subscriber) {
    _subscriptions.removeSubscription(subscriber, _subscriptions._nodes);
}

void MqttBroker::unsubscribe(MqttSubscriber& subscriber, std::vector<std::string> topics) {
    _subscriptions.removeSubscription(subscriber, topics, _subscriptions._nodes);
}

void MqttBroker::publish(std::string topic, std::vector<ubyte> payload) {
    std::deque<std::string> topParts;
    boost::split(topParts, topic, boost::is_any_of("/"));
    publish(topic, topParts, payload);
}

void MqttBroker::publish(std::string topic, std::deque<std::string> topParts,
                         std::vector<ubyte> payload) {
    _subscriptions.publish(topic, topParts, payload);
}
