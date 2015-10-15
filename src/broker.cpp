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
                           TopicParts topicParts):
    _subscriber(subscriber),
    _part(std::move(topicParts.back())),
    _topic(std::move(topic.topic)),
    _qos(topic.qos) {
}

void Subscription::newMessage(const std::string& topic, const std::vector<ubyte>& payload) {
    _subscriber.newMessage(topic, payload);
}

bool Subscription::isSubscriber(const MqttSubscriber& subscriber) const {
    return &_subscriber == &subscriber;
}

bool Subscription::isSubscription(const MqttSubscriber& subscriber,
                                  std::vector<std::string> topics) const {
    return isSubscriber(subscriber) && isTopic(topics);
}

bool Subscription::isTopic(const std::vector<std::string>& topics) const {
    return std::find(topics.cbegin(), topics.cend(), _topic) != topics.cend();
}

SubscriptionTree::SubscriptionTree():
    _useCache(false) {
}

void SubscriptionTree::addSubscription(Subscription* s, const TopicParts& parts) {
    assert(parts.size());
    clearCache();
    addSubscriptionImpl(s, parts, nullptr, _nodes);
}

//remove_if returns an iterator to the new end but actually leaves
//the container unchanged, which isn't what I want
template<typename T, typename F>
void removeIf(T& container, F pred) {
    const auto newEnd = std::remove_if(std::begin(container),
                                       std::end(container),
                                       pred);
    container.resize(newEnd - std::begin(container));
}

void SubscriptionTree::removeSubscription(MqttSubscriber& subscriber,
                                          std::unordered_map<std::string, NodePtr>& nodes) {
    clearCache();
    std::unordered_map<std::string, NodePtr> newNodes = nodes;
    for(auto n: newNodes) {
        if(n.second->leaves.size()) {
            removeIf(n.second->leaves, [&subscriber](Subscription* l) { return l->isSubscriber(subscriber); });
            if(!n.second->leaves.size() && !n.second->branches.size()) {
                removeNode(n.second->parent, n.second);
            }
        } else {
            removeSubscription(subscriber, n.second->branches);
        }
    }
}

void SubscriptionTree::removeSubscription(MqttSubscriber& subscriber,
                                          std::vector<std::string> topics,
                                          std::unordered_map<std::string, NodePtr>& nodes) {
    clearCache();
    std::unordered_map<std::string, NodePtr> newNodes = nodes;
    for(auto n: newNodes) {
        if(n.second->leaves.size()) {
            removeIf(n.second->leaves, [&subscriber, &topics](Subscription* l) {
                    return l->isSubscription(subscriber, topics); });
            if(!n.second->leaves.size() && !n.second->branches.size()) {
                removeNode(n.second->parent, n.second);
            }
        } else {
            removeSubscription(subscriber, topics, n.second->branches);
        }
    }
}


void SubscriptionTree::publish(std::string topic, TopicParts topParts,
                               const std::vector<ubyte>& payload) {
    publish(topic, topParts.cbegin(), topParts.cend(), payload, _nodes);
}

void SubscriptionTree::publish(std::string topic,
                               TopicParts::const_iterator topPartsBegin,
                               TopicParts::const_iterator topPartsEnd,
                               const std::vector<ubyte>& payload,
                               std::unordered_map<std::string, NodePtr>& nodes) {
    //check the cache first
    if(_useCache && _cache.count(topic)) {
        for(auto s: _cache[topic]) s->newMessage(topic, payload);
        return;
    }

    const auto size = (topPartsEnd - topPartsBegin);
    //not in the cache or not using the cache, do it the hard way
    for(auto part: std::vector<std::string>{*topPartsBegin, "#", "+"}) {
        if(nodes.count(part)) {
            if(size == 1 && nodes[part]->branches.count("#")) {
                //So that "finance/#" matches finance
                publishLeaves(topic, payload, topPartsBegin, topPartsEnd, nodes[part]->branches["#"]->leaves);
            }
            publishLeaves(topic, payload, topPartsBegin, topPartsEnd, nodes[part]->leaves);
            if(size > 1) {
                publish(topic, topPartsBegin + 1, topPartsEnd, payload, nodes[part]->branches);
            }
        }
    }
}

void SubscriptionTree::publishLeaves(std::string topic, const std::vector<ubyte>& payload,
                                     TopicParts::const_iterator topPartsBegin,
                                     TopicParts::const_iterator topPartsEnd,
                                     std::vector<Subscription*> subscriptions) {
    for(auto sub: subscriptions) {
        if((topPartsEnd - topPartsBegin) == 1 &&
           equalOrPlus(sub->_part, *topPartsBegin)) {
            publishLeaf(sub, topic, payload);
        }
        else if(sub->_part == "#") {
            publishLeaf(sub, topic, payload);
        }
    }
}

void SubscriptionTree::publishLeaf(Subscription* sub, std::string topic, const std::vector<ubyte>& payload) {
    sub->newMessage(topic, payload);
    if(_useCache) _cache[topic].push_back(sub);
}


void SubscriptionTree::addSubscriptionImpl(Subscription* s,
                                           TopicParts parts,
                                           NodePtr parent,
                                           std::unordered_map<std::string, NodePtr>& nodes) {
    auto part = parts.front();
    parts.pop_front();
    auto node = addOrFindNode(part, parent, nodes);
    if(!parts.size()) {
        node->leaves.emplace_back(s);
    } else {
        addSubscriptionImpl(s, parts, node, node->branches);
    }
}


auto SubscriptionTree::addOrFindNode(std::string part, NodePtr parent,
                                     std::unordered_map<std::string, NodePtr>& nodes) -> NodePtr {
    if(nodes.count(part) && part == nodes[part]->part) {
        return nodes[part];
    }
    auto node = new Node(part, parent);
    nodes[part] = node;
    return node;
}


void SubscriptionTree::removeNode(NodePtr parent, NodePtr child) {
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
        TopicParts parts;
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

void MqttBroker::publish(const std::string& topic, const std::string& payload) {
    std::vector<ubyte> realPayload(payload.begin(), payload.end());
    publish(topic, realPayload);
}

void MqttBroker::publish(const std::string& topic, const std::vector<ubyte>& payload) {
    TopicParts topParts;
    boost::split(topParts, topic, boost::is_any_of("/"));
    publish(topic, topParts, payload);
}

void MqttBroker::publish(const std::string& topic,
                         const TopicParts& topParts,
                         const std::vector<ubyte>& payload) {
    _subscriptions.publish(topic, topParts, payload);
}
